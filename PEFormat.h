
#ifndef PEFORMAT_H
#define PEFORMAT_H

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace PE
{
    struct DOSHeader
    {
        unsigned char    _unusedBytes[60];
        unsigned long    offsetOfNTSignature;
    };

    struct NTFileHeader
    {
        unsigned short   targetMachineArchitecture;
        unsigned short   numberOfSections;
        unsigned char    _unusedBytes1[12];
        unsigned short   sizeOfOptionalHeader;
        unsigned char    _unusedBytes2[2];
    };

    struct NTOptionalHeader64
    {
        unsigned short        peSignature;
        unsigned char         linkerMajorVersion;
        unsigned char         linkerMinorVersion;
        unsigned long         sizeOfCodeInBytes;
        unsigned long         sizeOfInitializedDataInBytes;
        unsigned long         sizeOfUninitializedDataInBytes;
        unsigned long         addressOfEntryPoint;
        unsigned long         addressOfBaseOfCode;
        unsigned long long    preferredBaseAddressOfImage;
        unsigned char         _unusedBytes[76];
        unsigned long         numberOfDataDirectories;
    };

    struct DataDirectoryEntry
    {
        unsigned long    dataDirectoryRVA;
        unsigned long    sizeInBytes;
    };

    struct SectionHeader
    {
        unsigned long long    sectionNameAsNumber;
        unsigned long         sectionSizeInBytesInMemory;
        unsigned long         sectionBaseAddressInMemory;
        unsigned long         sizeOfRawDataInBytes;
        unsigned long         pointerToRawData;
        unsigned long         pointerToRelocations;
        unsigned long         pointerToLineNumbers;
        unsigned short        numberOfRelocations;
        unsigned short        numberOfLineNumberEntries;
        unsigned long         sectionCharacteristics;
    };

    struct ExportedFunction
    {
        std::string    name;
    };

    DOSHeader
    extractDOSHeader( unsigned char const* rawBytesFromStartOfDOSHeader );

    NTFileHeader
    extractNTFileHeader( unsigned char const* rawBytesFromStartOfNTFileHeader );

    NTOptionalHeader64
    extract64bitNTOptionalHeader( unsigned char const* rawBytesFromStartOfNTOptionalHeader );

    std::vector<DataDirectoryEntry>
    extractDataDirectoryEntries( unsigned char const* rawBytesFromStartOfDataDirectories,
                                 NTOptionalHeader64 const& ntOptionalHeader );

    std::map<std::string, SectionHeader>
    extractSectionHeaders( unsigned char const* rawBytesFromStartOfSectionHeaders,
                           int const numberOfSections );

    std::map<std::string, std::vector<unsigned char>>
    extractRawSectionContents( unsigned char const* rawBytesFromStartOfFile,
                               std::map<std::string, SectionHeader> const& sectionHeaders );

    std::optional<std::map<std::string, std::vector<std::string>>>
    extractImportedFunctionsInfo( std::vector<DataDirectoryEntry> const& dataDirectoryEntries,
                                  std::map<std::string, SectionHeader> const& sectionHeaders,
                                  std::map<std::string, std::vector<unsigned char>> const& sectionRawData );

    std::optional<std::vector<ExportedFunction>>
    extractExportedFunctionsInfo( std::vector<DataDirectoryEntry> const& dataDirectoryEntries,
                                  std::map<std::string, SectionHeader> const& sectionHeaders,
                                  std::map<std::string, std::vector<unsigned char>> const& sectionRawData );

    std::string
    getMachineArchitectureName( unsigned short const machineArchitecture );

    std::string
    getPESignatureName( unsigned short const peSignature );

    std::string
    getImageDataDirectoryDescription( unsigned long const dataDirectoryIndex );
}

#endif // PEFORMAT_H