
#ifndef EXEFILE_H
#define EXEFILE_H

#include <string>
#include <vector>

struct DOSHeader
{
    unsigned char    _unusedBytes[60];
    unsigned long    offsetOfNTFileHeader;
};

struct NTFileHeader
{
    unsigned char    _unusedBytes1[16];
    unsigned short   sizeOfOptionalHeader;
    unsigned char    _unusedBytes2[2];
};

struct NTOptionalHeader64
{
    unsigned short   peSignature;
    unsigned char    _unusedBytes[222];
};

struct EXEFile
{
    std::vector<unsigned char>    rawBytes;
    DOSHeader                     dosHeader;
    unsigned long                 ntSignature;
    NTFileHeader                  ntFileHeader;
    NTOptionalHeader64            ntOptionalHeader;
};

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile );

#endif // EXEFILE_H