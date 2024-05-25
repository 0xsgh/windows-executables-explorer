
#ifndef EXEFILE_H
#define EXEFILE_H

#include <map>
#include <string>
#include <vector>

struct DOSHeader
{
    unsigned char    _unusedBytes[60];
    unsigned long    offsetOfNTFileHeader;
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

struct EXEFile
{
    std::vector<unsigned char>                           rawBytes;
    DOSHeader                                            dosHeader;
    unsigned long                                        ntSignature;
    NTFileHeader                                         ntFileHeader;
    NTOptionalHeader64                                   ntOptionalHeader;
    std::vector<DataDirectoryEntry>                      dataDirectoryEntries;
    std::map<std::string, SectionHeader>                 sectionHeadersNameToInfo;
    std::map<std::string, std::vector<unsigned char>>    sectionNameToRawData;
};

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile );

std::string
getMachineArchitectureName( unsigned short const machineArchitecture );

std::string
getPESignatureName( unsigned short const peSignature );

std::string
getImageDataDirectoryDescription( unsigned long const dataDirectoryIndex );

#endif // EXEFILE_H