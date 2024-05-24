
#include "EXEFile.h"

#include <cstring>
#include <fstream>

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

    std::memcpy( &loadedEXEFile.dosHeader,
                 loadedEXEFile.rawBytes.data(),
                 sizeof( DOSHeader ) );

    auto const ntFileHeaderOffset =
        loadedEXEFile.dosHeader.offsetOfNTFileHeader + sizeof( loadedEXEFile.ntSignature );
    std::memcpy( &loadedEXEFile.ntFileHeader,
                 loadedEXEFile.rawBytes.data() + ntFileHeaderOffset,
                 sizeof( NTFileHeader ) );

    auto const ntOptionalHeaderOffset = ntFileHeaderOffset + sizeof( NTFileHeader );
    auto const optionalHeaderSignature =
        *reinterpret_cast<unsigned short const*>( loadedEXEFile.rawBytes.data() +
                                                  ntOptionalHeaderOffset );
    if ( optionalHeaderSignature != optionalHeaderSig_PE32Plus )
    {
        throw std::runtime_error{ "Only PE32+ files are supported." };
    }

    std::memcpy( &loadedEXEFile.ntOptionalHeader,
                 loadedEXEFile.rawBytes.data() + ntOptionalHeaderOffset,
                 sizeof( NTOptionalHeader64 ) );

    return loadedEXEFile;
}