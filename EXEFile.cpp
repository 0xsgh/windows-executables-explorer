
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

    auto const dataDirectoryEntriesOffset =
        ntOptionalHeaderOffset + sizeof( NTOptionalHeader64 );
    auto const numberOfDataDirectories =
        loadedEXEFile.ntOptionalHeader.numberOfDataDirectories;
    loadedEXEFile.dataDirectoryEntries.resize( numberOfDataDirectories );
    std::memcpy( loadedEXEFile.dataDirectoryEntries.data(),
                 loadedEXEFile.rawBytes.data() + dataDirectoryEntriesOffset,
                 numberOfDataDirectories * sizeof( DataDirectoryEntry ) );

    return loadedEXEFile;
}

std::string
getMachineArchitectureName( unsigned short const machineArchitecture )
{
    switch ( machineArchitecture )
    {
        case 0x014C:
            return "Intel 386";
        case 0x0200:
            return "Intel Itanium";
        case 0x8664:
            return "AMD64";
        default:
            return "<Unknown architecture>";
    }
}

std::string
getPESignatureName( unsigned short const peSignature )
{
    switch ( peSignature )
    {
        case 0x010B:
            return "PE32";
        case 0x020B:
            return "PE32+";
        case 0x107:
            return "ROM image";
        default:
            return "<Unknown signature>";
    }
}

std::string
getImageDataDirectoryDescription( unsigned long const dataDirectoryIndex )
{
    switch ( dataDirectoryIndex )
    {
        case 0:
            return "Export Directory";
        case 1:
            return "Import Directory";
        case 2:
            return "Resource Directory";
        case 3:
            return "Exception Directory";
        case 4:
            return "Security Directory";
        case 5:
            return "Base Relocation Table";
        case 6:
            return "Debug Directory";
        case 7:
            return "Architecture-Specific Data";
        case 8:
            return "RVA of Global Pointer Register";
        case 9:
            return "Thread-Local Storage Directory";
        case 10:
            return "Load Configuration Directory";
        case 11:
            return "Bound Import Directory in headers";
        case 12:
            return "Import Address Table";
        case 13:
            return "Delay Import Descriptor";
        case 14:
            return "CLR header";
        case 15:
            return "Reserved";
        default:
            return "<Unknown data directory>";
    }
}