
#include "EXEFile.h"

#include <cstring>
#include <fstream>
#include <optional>
#include <string>

namespace
{
    auto const optionalHeaderSig_PE32Plus = 0x20B;
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

    loadedEXEFile.sectionHeadersNameToInfo =
        PE::extractSectionHeaders( loadedEXEFile.rawBytes.data() + sectionHeaderTableOffset,
                                   numberOfSections );

    loadedEXEFile.sectionNameToRawData =
        PE::extractRawSectionContents( loadedEXEFile.rawBytes.data(),
                                       loadedEXEFile.sectionHeadersNameToInfo );

    auto importedDLLToImportedFunctions =
        PE::extractImportedFunctionsInfo( loadedEXEFile.dataDirectoryEntries,
                                          loadedEXEFile.sectionHeadersNameToInfo,
                                          loadedEXEFile.sectionNameToRawData );
    if ( importedDLLToImportedFunctions )
    {
        loadedEXEFile.importedDLLToImportedFunctions = *importedDLLToImportedFunctions;
    }

    auto exportedFunctionsInfo =
        PE::extractExportedFunctionsInfo( loadedEXEFile.dataDirectoryEntries,
                                          loadedEXEFile.sectionHeadersNameToInfo,
                                          loadedEXEFile.sectionNameToRawData );
    if ( exportedFunctionsInfo )
    {
        loadedEXEFile.exportedFunctions = *exportedFunctionsInfo;
    }

    return loadedEXEFile;
}