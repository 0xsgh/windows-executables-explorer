
#ifndef EXEFILE_H
#define EXEFILE_H

#include <string>
#include <vector>

struct EXEFile
{
    std::vector<unsigned char>    rawBytes;
};

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile );

#endif // EXEFILE_H