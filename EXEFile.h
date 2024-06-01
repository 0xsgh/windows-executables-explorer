
#ifndef EXEFILE_H
#define EXEFILE_H

#include "PEFile.h"

#include <map>
#include <string>
#include <vector>

struct EXEFile
{
    std::vector<unsigned char>                           rawBytes;
    PE::DOSHeader                                        dosHeader;
    unsigned long                                        ntSignature;
    PE::NTFileHeader                                     ntFileHeader;
    PE::NTOptionalHeader64                               ntOptionalHeader;
    std::vector<PE::DataDirectoryEntry>                  dataDirectoryEntries;
    std::map<std::string, PE::SectionHeader>             sectionHeadersNameToInfo;
    std::map<std::string, std::vector<unsigned char>>    sectionNameToRawData;
    std::map<std::string, std::vector<std::string>>      importedDLLToImportedFunctions;
    std::vector<PE::ExportedFunction>                    exportedFunctions;
};

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile );

#endif // EXEFILE_H