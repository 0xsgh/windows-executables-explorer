
#include "EXEFile.h"

#include <cstring>
#include <fstream>
#include <optional>
#include <string>

namespace
{
    auto const optionalHeaderSig_PE32Plus = 0x20B;

    auto const importTableIdx = 1;

    struct ImportDirectoryTableEntry
    {
        unsigned long    importLookupTableRVA;
        unsigned long    timestamp;
        unsigned long    forwarderChainIdx;
        unsigned long    namestringRVA;
        unsigned long    importAddressTableRVA;
    };

    struct ImportLookupTableEntry64
    {
        unsigned long long    ordinalNumberOrNameTableRVA: 63;
        unsigned long long    isOrdinal: 1;
    };

    std::string
    getEXESectionName( unsigned long long const sectionNameAsNumber );

    std::optional<std::string>
    getNameOfSectionContainingRVA( EXEFile const& exeFile,
                                   unsigned long long const rvaOfInterest );
}

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile )
{
    auto loadedEXEFile = EXEFile{};

    {
        auto exeFile = std::ifstream{ pathOfExecutableFile, std::ios::binary };

        if( !exeFile.is_open() )
        {
            throw std::runtime_error{ "Failed to open '" +
                                      pathOfExecutableFile +
                                      "'." };
        }

        auto const fileSizeInBytes =
            static_cast<int>( exeFile.seekg( 0, std::ios::end ).tellg() );
        exeFile.seekg( 0, std::ios::beg );

        loadedEXEFile.rawBytes.resize( fileSizeInBytes );
        exeFile.read( reinterpret_cast<char*>( loadedEXEFile.rawBytes.data() ),
                      fileSizeInBytes );
    }

    std::memcpy( &loadedEXEFile.dosHeader,
                 loadedEXEFile.rawBytes.data(),
                 sizeof( DOSHeader ) );

    auto const ntFileHeaderOffset =
        loadedEXEFile.dosHeader.offsetOfNTFileHeader + sizeof( loadedEXEFile.ntSignature );
    std::memcpy( &loadedEXEFile.ntFileHeader,
                 loadedEXEFile.rawBytes.data() + ntFileHeaderOffset,
                 sizeof( NTFileHeader ) );

    auto const ntOptionalHeaderOffset = ntFileHeaderOffset + sizeof( NTFileHeader );
    auto const optionalHeaderSignature =
        *reinterpret_cast<unsigned short const*>( loadedEXEFile.rawBytes.data() +
                                                  ntOptionalHeaderOffset );
    if ( optionalHeaderSignature != optionalHeaderSig_PE32Plus )
    {
        throw std::runtime_error{ "Only PE32+ files are supported." };
    }

    std::memcpy( &loadedEXEFile.ntOptionalHeader,
                 loadedEXEFile.rawBytes.data() + ntOptionalHeaderOffset,
                 sizeof( NTOptionalHeader64 ) );

    auto const dataDirectoryEntriesOffset =
        ntOptionalHeaderOffset + sizeof( NTOptionalHeader64 );
    auto const numberOfDataDirectories =
        loadedEXEFile.ntOptionalHeader.numberOfDataDirectories;
    loadedEXEFile.dataDirectoryEntries.resize( numberOfDataDirectories );
    std::memcpy( loadedEXEFile.dataDirectoryEntries.data(),
                 loadedEXEFile.rawBytes.data() + dataDirectoryEntriesOffset,
                 numberOfDataDirectories * sizeof( DataDirectoryEntry ) );

    auto const sectionHeaderTableOffset =
        dataDirectoryEntriesOffset + numberOfDataDirectories * sizeof( DataDirectoryEntry );
    auto const numberOfSections = loadedEXEFile.ntFileHeader.numberOfSections;
    for ( auto i = 0; i < numberOfSections; i++ )
    {
        auto const& sectionHeader =
            *reinterpret_cast<SectionHeader const*>( loadedEXEFile.rawBytes.data() +
                                                     sectionHeaderTableOffset +
                                                     i * sizeof( SectionHeader ) );
        auto const sectionName = getEXESectionName( sectionHeader.sectionNameAsNumber );

        loadedEXEFile.sectionHeadersNameToInfo[sectionName] = sectionHeader;

        auto const rawDataOffset = sectionHeader.pointerToRawData;
        auto const rawDataSize = sectionHeader.sizeOfRawDataInBytes;

        if ( rawDataOffset + rawDataSize > loadedEXEFile.rawBytes.size() )
        {
            throw std::runtime_error{ "Section '" + sectionName +
                                      "' has invalid raw data offset and/or size." };
        }

        loadedEXEFile.sectionNameToRawData[sectionName].resize( rawDataSize );
        std::memcpy( loadedEXEFile.sectionNameToRawData.at( sectionName ).data(),
                     loadedEXEFile.rawBytes.data() + rawDataOffset,
                     rawDataSize );
    }

    if ( hasImportTable( loadedEXEFile ) )
    {
        auto hostSectionName =
            getNameOfSectionContainingRVA( loadedEXEFile,
                                           loadedEXEFile.dataDirectoryEntries[importTableIdx].dataDirectoryRVA );

        if ( not hostSectionName )
        {
            throw std::runtime_error{ "Import table is not in any section." };
        }

        auto const& idataRawBytes = loadedEXEFile.sectionNameToRawData.at( *hostSectionName ).data();
        auto const& idataSectionHeader = loadedEXEFile.sectionHeadersNameToInfo.at( *hostSectionName );
        auto const idataSectionRVA = idataSectionHeader.sectionBaseAddressInMemory;
        auto const importDirectoryTableOffset =
            loadedEXEFile.dataDirectoryEntries[importTableIdx].dataDirectoryRVA - idataSectionRVA;
        auto const& importDirectoryTable =
            reinterpret_cast<ImportDirectoryTableEntry const*>( idataRawBytes +
                                                                importDirectoryTableOffset );
        for ( auto i = 0;; i++ )
        {
            if (    importDirectoryTable[i].importLookupTableRVA == 0
                and importDirectoryTable[i].timestamp == 0
                and importDirectoryTable[i].forwarderChainIdx == 0
                and importDirectoryTable[i].namestringRVA == 0
                and importDirectoryTable[i].importAddressTableRVA == 0 )
            {
                break;
            }

            auto const importedDLLName =
                std::string( reinterpret_cast<char const*>( idataRawBytes +
                                                            importDirectoryTable[i].namestringRVA -
                                                            idataSectionRVA ) );

            auto const importLookupTableOffset =
                importDirectoryTable[i].importLookupTableRVA - idataSectionRVA;
            auto const& importLookupTable =
                reinterpret_cast<ImportLookupTableEntry64 const*>( idataRawBytes +
                                                                importLookupTableOffset );
            for ( auto j = 0;; j++ )
            {
                if (     importLookupTable[j].ordinalNumberOrNameTableRVA == 0
                    and importLookupTable[j].isOrdinal == 0 )
                {
                    break;
                }

                auto const importedFunctionName =
                    std::string( reinterpret_cast<char const*>( idataRawBytes +
                                                                importLookupTable[j].ordinalNumberOrNameTableRVA +
                                                                sizeof( unsigned short ) -
                                                                idataSectionRVA ) );
                loadedEXEFile.importedDLLToImportedFunctions[importedDLLName].push_back( importedFunctionName );
            }
        }
    }

    return loadedEXEFile;
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

bool
hasImportTable( EXEFile const& exeFile )
{
    return exeFile.dataDirectoryEntries[importTableIdx].dataDirectoryRVA != 0;
}

namespace
{
    std::string
    getEXESectionName( unsigned long long const sectionNameAsNumber )
    {
        auto numberFormHasNull = false;
        for ( auto i = 0; i < sizeof( sectionNameAsNumber ); i++ )
        {
            if ( ( reinterpret_cast<char const*>( &sectionNameAsNumber )[i] ) == '\0' )
            {
                numberFormHasNull = true;
                break;
            }
        }

        if ( not numberFormHasNull )
        {
            return std::string( reinterpret_cast<char const*>( &sectionNameAsNumber ),
                                sizeof( sectionNameAsNumber ) );
        }
        else
        {
            return std::string( reinterpret_cast<char const*>( &sectionNameAsNumber ) );
        }
    }

    std::optional<std::string>
    getNameOfSectionContainingRVA( EXEFile const& exeFile,
                                   unsigned long long const rvaOfInterest )
    {
        for ( auto const& [sectionName, sectionHeader] : exeFile.sectionHeadersNameToInfo )
        {
            if (    rvaOfInterest >= sectionHeader.sectionBaseAddressInMemory
                and rvaOfInterest < sectionHeader.sectionBaseAddressInMemory + sectionHeader.sectionSizeInBytesInMemory )
            {
                return sectionName;
            }
        }

        return std::nullopt;
    }
}