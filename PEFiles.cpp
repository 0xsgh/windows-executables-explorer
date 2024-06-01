
#include "PEFiles.h"

#include <cstring>
#include <fstream>
#include <optional>
#include <string>

namespace
{
    auto const optionalHeaderSig_PE32Plus = 0x20B;

    std::vector<unsigned char>
    loadPEFileAsRawBytes( std::string const& pathOfPEFileToLoad )
    {
        auto peFile = std::ifstream{ pathOfPEFileToLoad, std::ios::binary };

        if( !peFile.is_open() )
        {
            throw std::runtime_error{ "Failed to open '" + pathOfPEFileToLoad + "'." };
        }

        auto const fileSizeInBytes =
            static_cast<int>( peFile.seekg( 0, std::ios::end ).tellg() );
        peFile.seekg( 0, std::ios::beg );

        auto rawBytes = std::vector<unsigned char>( fileSizeInBytes );

        peFile.read( reinterpret_cast<char*>( rawBytes.data() ),
                     fileSizeInBytes );

        return rawBytes;
    }
}

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile )
{
    auto loadedEXEFile = EXEFile{};

    auto const rawBytes = loadPEFileAsRawBytes( pathOfExecutableFile );

    loadedEXEFile.dosHeader = PE::extractDOSHeader( rawBytes.data() );

    auto const ntFileHeaderOffset =
        loadedEXEFile.dosHeader.offsetOfNTSignature +
        sizeof( loadedEXEFile.dosHeader.offsetOfNTSignature );
    loadedEXEFile.ntFileHeader =
        PE::extractNTFileHeader( rawBytes.data() + ntFileHeaderOffset );

    auto const ntOptionalHeaderOffset = ntFileHeaderOffset + sizeof( PE::NTFileHeader );
    loadedEXEFile.ntOptionalHeader =
        PE::extract64bitNTOptionalHeader( rawBytes.data() +
                                          ntOptionalHeaderOffset );

    if ( loadedEXEFile.ntOptionalHeader.peSignature != optionalHeaderSig_PE32Plus )
    {
        throw std::runtime_error{ "Only PE32+ files are supported." };
    }

    auto const dataDirectoryEntriesOffset =
        ntOptionalHeaderOffset + sizeof( PE::NTOptionalHeader64 );
    loadedEXEFile.dataDirectoryEntries =
        PE::extractDataDirectoryEntries( rawBytes.data() + dataDirectoryEntriesOffset,
                                         loadedEXEFile.ntOptionalHeader );

    auto const sectionHeaderTableOffset =
        dataDirectoryEntriesOffset +
        loadedEXEFile.dataDirectoryEntries.size() * sizeof( PE::DataDirectoryEntry );
    auto const numberOfSections = loadedEXEFile.ntFileHeader.numberOfSections;

    loadedEXEFile.sectionHeadersNameToInfo =
        PE::extractSectionHeaders( rawBytes.data() + sectionHeaderTableOffset,
                                   numberOfSections );

    loadedEXEFile.sectionNameToRawData =
        PE::extractRawSectionContents( rawBytes.data(),
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

OBJFile
loadOBJFile( std::string const& pathOfObjectFile )
{
    auto loadedOBJFile = OBJFile{};

    auto const rawBytes = loadPEFileAsRawBytes( pathOfObjectFile );

    loadedOBJFile.ntFileHeader = PE::extractNTFileHeader( rawBytes.data() );

    loadedOBJFile.sectionHeaders =
        PE::extractSectionHeadersFromOBJFile( rawBytes.data() + sizeof( PE::NTFileHeader ),
                                              loadedOBJFile.ntFileHeader.numberOfSections );

    return loadedOBJFile;
}