#include "fflags.hpp"

// vendor
#include <nlohmann/json.hpp>

namespace odessa::engine
{
    c_fflags::c_fflags( ) noexcept
    {
        std::uint64_t singleton = 0;

        std::ifstream file( "address.json" );
        if ( file )
        {
            nlohmann::json json;
            file >> json;

            if ( json.contains( "singleton" ) )
                singleton = json[ "singleton" ].get< std::uint64_t >( );
        }

        if ( singleton )
        {
            auto rebased = g_memory->rebase( singleton, e_rebase_type::add );
            auto pointer = g_memory->read< std::uint64_t >( rebased );

            const auto first  = g_memory->read< std::uint32_t >( pointer + 0x30 );
            const auto second = g_memory->read< std::uint32_t >( pointer + 0x38 );

            if ( first == 0x7fff and second == 0x8000 )
            {
                std::println( "found singleton [cached]" );
                m_singleton = pointer;
                return;
            }
        }

        const auto result = g_memory->find( constants::pattern );
        if ( !result )
        {
            std::println( "failed to find pattern" );
            return;
        }

        const auto instruction = g_memory->read( result + 4, 7 );
        const auto relative    = *reinterpret_cast< const std::int32_t * >( instruction.data( ) + 3 );

        const auto target = ( result + 4 ) + 7 + relative;

        const auto rebased  = g_memory->rebase( target );
        const auto absolute = g_memory->read< std::uint64_t >( target );

        singleton = rebased;

        nlohmann::json json;
        json[ "singleton" ] = rebased;

        std::ofstream out_file( "address.json" );
        out_file << json.dump( 4 );

        std::println( "found singleton [pattern]" );
        m_singleton = absolute;
    }

    c_remote_fflag c_fflags::find( const std::string &name ) noexcept
    {
        if ( !m_singleton )
            return c_remote_fflag { 0 };

        std::uint64_t hash = m_hash;
        for ( const auto &character : name )
        {
            hash ^= static_cast< std::uint8_t >( character );
            hash *= m_prime;
        }

        std::uint64_t bucket_mask { 0 };
        std::uint64_t bucket_list { 0 };

        while ( true )
        {
            bucket_mask = g_memory->read< std::uint64_t >( m_singleton + 0x30 );
            bucket_list = g_memory->read< std::uint64_t >( m_singleton + 0x18 );

            if ( bucket_mask != 0 and bucket_list != 0 )
                break;

            Sleep( 100 );
        }

        const auto end_node = g_memory->read< std::uint64_t >( m_singleton + sizeof( void * ) );

        const auto bucket_index = hash & bucket_mask;
        auto current_node = g_memory->read< std::uint64_t >( bucket_list + bucket_index * ( sizeof( void * ) * 2 ) + sizeof( void * ) );

        if ( current_node == end_node )
            return c_remote_fflag { 0 };

        const auto first_node_in_bucket = g_memory->read< std::uint64_t >( bucket_list + bucket_index * ( sizeof( void * ) * 2 ) );

        while ( true )
        {
            const auto node_name_length = g_memory->read< std::uint64_t >( current_node + 0x20 );
            if ( node_name_length == name.length( ) )
            {
                const auto    name_capacity = g_memory->read< std::uint64_t >( current_node + 0x28 );
                std::uint64_t name_ptr      = current_node + 0x10;

                if ( name_capacity >= ( sizeof( void * ) * 2 ) )
                    name_ptr = g_memory->read< std::uint64_t >( name_ptr );

                const auto        name_buffer = g_memory->read( name_ptr, name.length( ) );
                const std::string node_name( name_buffer.begin( ), name_buffer.end( ) );

                if ( name == node_name )
                {
                    const auto address = g_memory->read< std::uint64_t >( current_node + 0x30 );
                    return c_remote_fflag { address };
                }
            }

            if ( current_node == first_node_in_bucket )
                break;

            current_node = g_memory->read< std::uint64_t >( current_node + sizeof( void * ) );
        }

        return c_remote_fflag { 0 };
    }
} // namespace odessa::engine