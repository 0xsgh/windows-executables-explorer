
#include "PEFormat.h"

#include <cstring>

namespace
{
    auto const exportTableIdx = 0;
    auto const importTableIdx = 1;

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

    std::optional<std::string>
    getNameOfSectionContainingRVA( std::map<std::string, PE::SectionHeader> const& sectionNameToHeader,
                                   unsigned long long const rvaOfInterest )
    {
        for ( auto const& [sectionName, sectionHeader] : sectionNameToHeader )
        {
            if (    rvaOfInterest >= sectionHeader.sectionBaseAddressInMemory
                and rvaOfInterest < sectionHeader.sectionBaseAddressInMemory + sectionHeader.sectionSizeInBytesInMemory )
            {
                return sectionName;
            }
        }

        return std::nullopt;
    }

    bool
    hasImportTable( std::vector<PE::DataDirectoryEntry> const& dataDirectoryEntries )
    {
        return dataDirectoryEntries[importTableIdx].dataDirectoryRVA != 0 and
               dataDirectoryEntries[importTableIdx].sizeInBytes != 0;
    }

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

    bool
    hasExportTable( std::vector<PE::DataDirectoryEntry> const& dataDirectoryEntries )
    {
        return dataDirectoryEntries[exportTableIdx].dataDirectoryRVA != 0 and
               dataDirectoryEntries[exportTableIdx].sizeInBytes != 0;
    }

    struct ExportDirectoryTableEntry
    {
        unsigned long     _reserved1;
        unsigned long     timestamp;
        unsigned short    dllMajorVersion;
        unsigned short    dllMinorVersion;
        unsigned long     namestringRVA;
        unsigned long     baseOrdinalNumber;
        unsigned long     numberOfExportAddressTableEntries;
        unsigned long     numberOfNamePointerTableEntries;
        unsigned long     exportAddressTableRVA;
        unsigned long     namePointerTableRVA;
        unsigned long     ordinalTableRVA;
    };
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

    std::map<std::string, std::vector<unsigned char>>
    extractRawSectionContents( unsigned char const* rawBytesFromStartOfFile,
                               std::map<std::string, SectionHeader> const& sectionHeaders )
    {
        auto sectionNameToRawData = std::map<std::string, std::vector<unsigned char>>{};

        for ( auto const& [sectionName, sectionHeader] : sectionHeaders )
        {
            auto const sectionSizeInFile = sectionHeader.sizeOfRawDataInBytes;
            auto const sectionOffsetInFile = sectionHeader.pointerToRawData;

            sectionNameToRawData[sectionName].resize( sectionSizeInFile );

            std::memcpy( sectionNameToRawData.at( sectionName ).data(),
                         rawBytesFromStartOfFile + sectionOffsetInFile,
                         sectionSizeInFile );
        }

        return sectionNameToRawData;
    }

    std::optional<std::map<std::string, std::vector<std::string>>>
    extractImportedFunctionsInfo( std::vector<DataDirectoryEntry> const& dataDirectoryEntries,
                                  std::map<std::string, SectionHeader> const& sectionHeaders,
                                  std::map<std::string, std::vector<unsigned char>> const& sectionRawData )
    {
        if ( not hasImportTable( dataDirectoryEntries ) )
        {
            return std::nullopt;
        }

        auto hostSectionName =
            getNameOfSectionContainingRVA( sectionHeaders,
                                           dataDirectoryEntries[importTableIdx].dataDirectoryRVA );

        if ( not hostSectionName )
        {
            return std::nullopt;
        }

        auto const& hostSectionHeader = sectionHeaders.at( *hostSectionName );
        auto const hostSectionRawBytes = sectionRawData.at( *hostSectionName ).data();

        auto const importDirectoryTable =
            reinterpret_cast<ImportDirectoryTableEntry const*>
                ( hostSectionRawBytes +
                  dataDirectoryEntries[importTableIdx].dataDirectoryRVA -
                  hostSectionHeader.sectionBaseAddressInMemory );

        auto dllNameToImportedFunctionNames = std::map<std::string, std::vector<std::string>>{};

        for ( auto i = 0;; i++ )
        {
            if (     importDirectoryTable[i].importLookupTableRVA == 0
                 and importDirectoryTable[i].timestamp == 0
                 and importDirectoryTable[i].forwarderChainIdx == 0
                 and importDirectoryTable[i].namestringRVA == 0
                 and importDirectoryTable[i].importAddressTableRVA == 0 )
            {
                break;
            }

            auto const importedDLLName =
                std::string( reinterpret_cast<char const*>( hostSectionRawBytes +
                                                            importDirectoryTable[i].namestringRVA -
                                                            hostSectionHeader.sectionBaseAddressInMemory ) );

            auto const importLookupTable =
                reinterpret_cast<ImportLookupTableEntry64 const*>
                    ( hostSectionRawBytes +
                      importDirectoryTable[i].importLookupTableRVA -
                      hostSectionHeader.sectionBaseAddressInMemory );

            for ( auto j = 0;; j++ )
            {
                if (     importLookupTable[j].ordinalNumberOrNameTableRVA == 0
                     and importLookupTable[j].isOrdinal == 0 )
                {
                    break;
                }

                auto const importedFunctionName =
                    std::string( reinterpret_cast<char const*>( hostSectionRawBytes +
                                                                importLookupTable[j].ordinalNumberOrNameTableRVA +
                                                                sizeof( unsigned short ) -
                                                                hostSectionHeader.sectionBaseAddressInMemory ) );

                dllNameToImportedFunctionNames[importedDLLName].push_back( importedFunctionName );
            }
        }

        return dllNameToImportedFunctionNames;
    }

    std::optional<std::vector<ExportedFunction>>
    extractExportedFunctionsInfo( std::vector<DataDirectoryEntry> const& dataDirectoryEntries,
                                  std::map<std::string, SectionHeader> const& sectionHeaders,
                                  std::map<std::string, std::vector<unsigned char>> const& sectionRawData )
    {
        if ( not hasExportTable( dataDirectoryEntries ) )
        {
            return std::nullopt;
        }

        auto hostSectionName =
            getNameOfSectionContainingRVA( sectionHeaders,
                                           dataDirectoryEntries[exportTableIdx].dataDirectoryRVA );

        if ( not hostSectionName )
        {
            return std::nullopt;
        }

        auto const& hostSectionHeader = sectionHeaders.at( *hostSectionName );
        auto const hostSectionRawBytes = sectionRawData.at( *hostSectionName ).data();

        auto const& exportDirectoryTableSoleEntry =
            *reinterpret_cast<ExportDirectoryTableEntry const*>
                ( hostSectionRawBytes +
                  dataDirectoryEntries[exportTableIdx].dataDirectoryRVA -
                  hostSectionHeader.sectionBaseAddressInMemory );

        auto const namePointerTable =
            reinterpret_cast<unsigned long const*>( hostSectionRawBytes +
                                                    exportDirectoryTableSoleEntry.namePointerTableRVA -
                                                    hostSectionHeader.sectionBaseAddressInMemory );

        auto exportedFunctionsInfo = std::vector<ExportedFunction>{};

        for ( auto i = 0; i < exportDirectoryTableSoleEntry.numberOfNamePointerTableEntries; i++ )
        {
            auto const exportedFunctionName =
                std::string( reinterpret_cast<char const*>( hostSectionRawBytes +
                                                            namePointerTable[i] -
                                                            hostSectionHeader.sectionBaseAddressInMemory ) );

            exportedFunctionsInfo.push_back( ExportedFunction
                                             {
                                                .name = exportedFunctionName
                                             } );
        }

        return exportedFunctionsInfo;
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