
#include "PEFile.h"

#include <cstring>

namespace
{
    std::string
    getSectionName( unsigned long long const sectionNameAsNumber )
    {
        auto numberRepresentationHasNull = false;
        for ( auto i = 0; i < sizeof( sectionNameAsNumber ); i++ )
        {
            if ( ( reinterpret_cast<char const*>( &sectionNameAsNumber )[i] ) == '\0' )
            {
                numberRepresentationHasNull = true;
                break;
            }
        }

        if ( not numberRepresentationHasNull )
        {
            return std::string( reinterpret_cast<char const*>( &sectionNameAsNumber ),
                                sizeof( sectionNameAsNumber ) );
        }
        else
        {
            return std::string( reinterpret_cast<char const*>( &sectionNameAsNumber ) );
        }
    }
}

namespace PE
{
    DOSHeader
    extractDOSHeader( unsigned char const* rawBytesFromStartOfDOSHeader )
    {
        return *reinterpret_cast<DOSHeader const*>( rawBytesFromStartOfDOSHeader );
    }

    NTFileHeader
    extractNTFileHeader( unsigned char const* rawBytesFromStartOfNTFileHeader )
    {
        return *reinterpret_cast<NTFileHeader const*>( rawBytesFromStartOfNTFileHeader );
    }

    NTOptionalHeader64
    extract64bitNTOptionalHeader( unsigned char const* rawBytesFromStartOfNTOptionalHeader )
    {
        return *reinterpret_cast<NTOptionalHeader64 const*>( rawBytesFromStartOfNTOptionalHeader );
    }

    std::vector<DataDirectoryEntry>
    extractDataDirectoryEntries( unsigned char const* rawBytesFromStartOfDataDirectories,
                                 NTOptionalHeader64 const& ntOptionalHeader )
    {
        auto dataDirectoryEntries = std::vector<DataDirectoryEntry>{};
        dataDirectoryEntries.resize( ntOptionalHeader.numberOfDataDirectories );

        std::memcpy( dataDirectoryEntries.data(),
                     rawBytesFromStartOfDataDirectories,
                     ntOptionalHeader.numberOfDataDirectories * sizeof( DataDirectoryEntry ) );

        return dataDirectoryEntries;
    }

    std::map<std::string, SectionHeader>
    extractSectionHeaders( unsigned char const* rawBytesFromStartOfSectionHeaders,
                           int const numberOfSections )
    {
        auto sectionNameToHeader = std::map<std::string, SectionHeader>{};

        for ( auto i = 0; i < numberOfSections; i++ )
        {
            auto const& sectionHeader =
                *reinterpret_cast<SectionHeader const*>( rawBytesFromStartOfSectionHeaders +
                                                         i * sizeof( SectionHeader ) );
            auto const sectionName = getSectionName( sectionHeader.sectionNameAsNumber );

            sectionNameToHeader[sectionName] = sectionHeader;
        }

        return sectionNameToHeader;
    }

    std::string
    getMachineArchitectureName( unsigned short const machineArchitecture )
    {
        switch ( machineArchitecture )
        {
            case 0x014C:
                return "Intel 386";
            case 0x0200:
                return "Intel Itanium";
            case 0x8664:
                return "AMD64";
            default:
                return "<Unknown architecture>";
        }
    }

    std::string
    getPESignatureName( unsigned short const peSignature )
    {
        switch ( peSignature )
        {
            case 0x010B:
                return "PE32";
            case 0x020B:
                return "PE32+";
            case 0x107:
                return "ROM image";
            default:
                return "<Unknown signature>";
        }
    }

    std::string
    getImageDataDirectoryDescription( unsigned long const dataDirectoryIndex )
    {
        switch ( dataDirectoryIndex )
        {
            case 0:
                return "Export Directory";
            case 1:
                return "Import Directory";
            case 2:
                return "Resource Directory";
            case 3:
                return "Exception Directory";
            case 4:
                return "Security Directory";
            case 5:
                return "Base Relocation Table";
            case 6:
                return "Debug Directory";
            case 7:
                return "Architecture-Specific Data";
            case 8:
                return "RVA of Global Pointer Register";
            case 9:
                return "Thread-Local Storage Directory";
            case 10:
                return "Load Configuration Directory";
            case 11:
                return "Bound Import Directory in headers";
            case 12:
                return "Import Address Table";
            case 13:
                return "Delay Import Descriptor";
            case 14:
                return "CLR header";
            case 15:
                return "Reserved";
            default:
                return "<Unknown data directory>";
        }
    }
}