#include <vector>
#include <filesystem>
#include <fstream>

namespace fgl::misc
{
    //Readfile from binary
    std::vector<char> read_file( std::filesystem::path path )
    {
        std::vector<char> buf;

        if( std::ifstream ifs( path.string(), std::ios::binary | std::ios::ate ); ifs )
        {
            ifs.exceptions( ifs.badbit | ifs.failbit | ifs.eofbit );
            buf.resize( static_cast< uint32_t >( ifs.tellg() ) );
            ifs.seekg( 0 );
            ifs.read( buf.data(), static_cast< std::streamsize >( buf.size() ) );
        }
        else
        {
            throw std::runtime_error( "Failed to open shader file: " + path.string() );
        }

        return buf;
    }
}