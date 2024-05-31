
#include "EXEFile.h"

#include <cstring>
#include <fstream>
#include <optional>
#include <string>

namespace
{
    auto const optionalHeaderSig_PE32Plus = 0x20B;

    auto const exportTableIdx = 0;
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

    loadedEXEFile.dosHeader = PE::extractDOSHeader( loadedEXEFile.rawBytes.data() );

    auto const ntFileHeaderOffset =
        loadedEXEFile.dosHeader.offsetOfNTSignature +
        sizeof( loadedEXEFile.dosHeader.offsetOfNTSignature );
    loadedEXEFile.ntFileHeader =
        PE::extractNTFileHeader( loadedEXEFile.rawBytes.data() + ntFileHeaderOffset );

    auto const ntOptionalHeaderOffset = ntFileHeaderOffset + sizeof( PE::NTFileHeader );
    loadedEXEFile.ntOptionalHeader =
        PE::extract64bitNTOptionalHeader( loadedEXEFile.rawBytes.data() +
                                          ntOptionalHeaderOffset );

    if ( loadedEXEFile.ntOptionalHeader.peSignature != optionalHeaderSig_PE32Plus )
    {
        throw std::runtime_error{ "Only PE32+ files are supported." };
    }

    auto const dataDirectoryEntriesOffset =
        ntOptionalHeaderOffset + sizeof( PE::NTOptionalHeader64 );
    loadedEXEFile.dataDirectoryEntries =
        PE::extractDataDirectoryEntries( loadedEXEFile.rawBytes.data() + dataDirectoryEntriesOffset,
                                         loadedEXEFile.ntOptionalHeader );

    auto const sectionHeaderTableOffset =
        dataDirectoryEntriesOffset +
        loadedEXEFile.dataDirectoryEntries.size() * sizeof( PE::DataDirectoryEntry );
    auto const numberOfSections = loadedEXEFile.ntFileHeader.numberOfSections;
    for ( auto i = 0; i < numberOfSections; i++ )
    {
        auto const& sectionHeader =
            *reinterpret_cast<PE::SectionHeader const*>( loadedEXEFile.rawBytes.data() +
                                                         sectionHeaderTableOffset +
                                                         i * sizeof( PE::SectionHeader ) );
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

    loadedEXEFile.sectionHeadersNameToInfo =
        PE::extractSectionHeaders( loadedEXEFile.rawBytes.data() + sectionHeaderTableOffset,
                                   numberOfSections );

    if ( hasImportTable( loadedEXEFile ) )
    {
        auto hostSectionName =
            getNameOfSectionContainingRVA( loadedEXEFile,
                                           loadedEXEFile.dataDirectoryEntries[importTableIdx].dataDirectoryRVA );

        if ( not hostSectionName )
        {
            throw std::runtime_error{ "Import table is not in any section." };
        }

        auto const& hostSectionRawBytes = loadedEXEFile.sectionNameToRawData.at( *hostSectionName ).data();
        auto const& hostSectionHeader = loadedEXEFile.sectionHeadersNameToInfo.at( *hostSectionName );
        auto const hostSectionRVA = hostSectionHeader.sectionBaseAddressInMemory;
        auto const importDirectoryTableOffset =
            loadedEXEFile.dataDirectoryEntries[importTableIdx].dataDirectoryRVA - hostSectionRVA;
        auto const& importDirectoryTable =
            reinterpret_cast<ImportDirectoryTableEntry const*>( hostSectionRawBytes +
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
                std::string( reinterpret_cast<char const*>( hostSectionRawBytes +
                                                            importDirectoryTable[i].namestringRVA -
                                                            hostSectionRVA ) );

            auto const importLookupTableOffset =
                importDirectoryTable[i].importLookupTableRVA - hostSectionRVA;
            auto const& importLookupTable =
                reinterpret_cast<ImportLookupTableEntry64 const*>( hostSectionRawBytes +
                                                                importLookupTableOffset );
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
                                                                hostSectionRVA ) );
                loadedEXEFile.importedDLLToImportedFunctions[importedDLLName].push_back( importedFunctionName );
            }
        }
    }

    if ( hasExportTable( loadedEXEFile ) )
    {
        auto hostSectionName =
            getNameOfSectionContainingRVA( loadedEXEFile,
                                           loadedEXEFile.dataDirectoryEntries[exportTableIdx].dataDirectoryRVA );

        if ( not hostSectionName )
        {
            throw std::runtime_error{ "Export table is not in any section." };
        }

        auto const& hostSectionRawBytes =
            loadedEXEFile.sectionNameToRawData.at( *hostSectionName ).data();
        auto const& hostSectionRVA =
            loadedEXEFile.sectionHeadersNameToInfo.at( *hostSectionName ).sectionBaseAddressInMemory;
        auto const exportDirectoryTableOffset =
            loadedEXEFile.dataDirectoryEntries[exportTableIdx].dataDirectoryRVA - hostSectionRVA;
        auto const& exportDirectoryTable =
            reinterpret_cast<ExportDirectoryTableEntry const*>( hostSectionRawBytes +
                                                                exportDirectoryTableOffset );

        auto const& namePointerTable =
            reinterpret_cast<unsigned long const*>( hostSectionRawBytes +
                                                    exportDirectoryTable->namePointerTableRVA -
                                                    hostSectionRVA );
        auto exportedFunctionNames = std::vector<std::string>{};
        for ( auto i = 0; i < exportDirectoryTable->numberOfNamePointerTableEntries; i++ )
        {
            auto const exportedFunctionName =
                std::string( reinterpret_cast<char const*>( hostSectionRawBytes +
                                                            namePointerTable[i] -
                                                            hostSectionRVA ) );
            loadedEXEFile.exportedFunctions.push_back( ExportedFunction
                                                       {
                                                           .name = std::string( exportedFunctionName )
                                                       } );
        }
    }

    return loadedEXEFile;
}

bool
hasImportTable( EXEFile const& exeFile )
{
    return exeFile.dataDirectoryEntries[importTableIdx].dataDirectoryRVA != 0;
}

bool
hasExportTable( EXEFile const& exeFile )
{
    return exeFile.dataDirectoryEntries[exportTableIdx].dataDirectoryRVA != 0;
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