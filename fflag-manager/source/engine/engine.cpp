#include "engine.hpp"

// vendor
#include <nlohmann/json.hpp>

// standard
#include <iostream>

namespace odessa::engine
{
    const std::vector< std::pair< const char *, std::pair< std::size_t, e_value_type > > > prefix_map = {
        { "DFString",  { 8, e_value_type::string } },
        {   "DFFlag",    { 6, e_value_type::flag } },
        {    "DFInt", { 5, e_value_type::integer } },
        {    "DFLog",     { 5, e_value_type::log } },
        {  "FString",  { 7, e_value_type::string } },
        {    "FFlag",    { 5, e_value_type::flag } },
        {     "FInt", { 4, e_value_type::integer } },
        {     "FLog",     { 4, e_value_type::log } }
    };

    bool string_to_bool( const std::string &string )
    {
        if ( string.empty( ) )
            return false;

        char first = static_cast< char >( std::tolower( string[ 0 ] ) );

        if ( first == 't' )
            return true;

        if ( first == 'f' )
            return false;

        if ( std::stoi( string ) != 0 )
            return true;

        return false;
    }

    std::int32_t level_to_integer( std::string string )
    {
        if ( const auto separator_pos = string.find_first_of( ",;" ); separator_pos != std::string::npos )
            string = string.substr( 0, separator_pos );

        string.erase( std::remove_if( string.begin( ), string.end( ), ::isspace ), string.end( ) );

        std::transform( string.begin( ), string.end( ), string.begin( ),
                        []( unsigned char c )
                        {
                            return std::tolower( c );
                        } );

        if ( string == "info" )
            return 6;

        if ( string == "warning" )
            return 4;

        if ( string == "error" )
            return 1;

        if ( string == "fatal" )
            return 0;

        if ( string == "verbose" )
            return 7;

        return std::stoi( string );
    }

    void setup( )
    {
        std::ifstream file( "fflags.json" );
        if ( !file.is_open( ) )
        {
            std::println( "failed to find fflags.json (doesn't exist)" );
            return;
        }

        nlohmann::json data;

        try
        {
            file >> data;
        }
        catch ( const nlohmann::json::parse_error &eggsception )
        {
            std::println( "failed to parse fflags.json: {}", eggsception.what( ) );
            return;
        };

        std::vector< std::string > failed;

        for ( const auto &[ key, value ] : data.items( ) )
        {
            e_value_type value_type { e_value_type::integer };
            std::string  name { key };

            bool prefix_found = false;

            for ( const auto &[ prefix, info ] : prefix_map )
            {
                if ( key.starts_with( prefix ) )
                {
                    name         = key.substr( info.first );
                    value_type   = info.second;
                    prefix_found = true;
                    break;
                }
            }

            if ( name.empty( ) )
                continue;

            const auto fflag = g_fflags->find( name );
            if ( !fflag )
            {
                failed.emplace_back( key );
                continue;
            }

            if ( fflag->value == reinterpret_cast< void * >( 0x65757254 ) or fflag->value == reinterpret_cast< void * >( 0x31303031 ) )
            {
                std::println( "fflag [{}] has unregistered getset, skipping", name );
                continue;
            }

            bool success = false;

            if ( value.is_boolean( ) )
            {
                const std::int32_t int_value = value.get< bool >( ) ? 1 : 0;
                success                      = fflag.set( int_value );

                std::println( "{} -> {}", name, success );
            }
            else if ( value.is_number_integer( ) )
            {
                success = fflag.set( value.get< std::int32_t >( ) );
                std::println( "{} -> {}", name, success );
            }
            else if ( value.is_string( ) )
            {
                const std::string str_value = value.get< std::string >( );

                switch ( value_type )
                {
                    case e_value_type::flag :
                    {
                        const std::int32_t int_value = string_to_bool( str_value ) ? 1 : 0;
                        success                      = fflag.set( int_value );
                        break;
                    }
                    case e_value_type::integer :
                    {
                        success = fflag.set( std::stoi( str_value ) );
                        break;
                    }
                    case e_value_type::string :
                    {
                        success = fflag.set( str_value );
                        break;
                    }
                    case e_value_type::log :
                    {
                        success = fflag.set( level_to_integer( str_value ) );
                        break;
                    }
                    default :
                    {
                        std::println( "can't determine type for {}", key );
                        continue;
                    }
                }

                std::println( "{} -> {} | {:#x}", name, success, reinterpret_cast< std::uint64_t >( fflag->value ) );
            }
            else
                std::println( "failed to parse type for key: {}", key );
        }

        if ( failed.empty( ) )
        {
            std::println( "============================" );
            std::println( "all fflags were set successfully" );
            return;
        }

        std::println( "============================" );
        std::println( "failed to set {} fflags", failed.size( ) );

        for ( const auto &idx : failed )
            std::println( "failed: {}", idx.c_str( ) );

        std::println( "============================" );
        std::println( "would you like to remove these missing flags from fflags.json? (y/n)" );

        std::string user_input;
        std::cin >> user_input;

        if ( user_input == "y" or user_input == "Y" )
        {
            for ( const auto &key_to_remove : failed )
                data.erase( key_to_remove );

            std::ofstream out_file( "fflags.json" );
            if ( out_file.is_open( ) )
            {
                out_file << data.dump( 4 );
                out_file.close( );

                std::println( "removed {} fflags from the list", failed.size( ) );
            }
            else
                std::println( "couldn't open fflags.json for writing" );
        }
    }
} // namespace odessa::engine
