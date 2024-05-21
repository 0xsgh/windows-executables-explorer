
#ifndef EXEFILE_H
#define EXEFILE_H

#include <string>
#include <vector>

struct EXEFile
{
    std::vector<unsigned char>    m_rawBytes;
};

EXEFile
loadEXEFile( std::string const& pathOfExecutableFile );

#endif // EXEFILE_H