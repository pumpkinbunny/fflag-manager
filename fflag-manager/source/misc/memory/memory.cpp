#include "memory.hpp"

// local->misc
#include "constants.hpp"

// standard
#include <tlhelp32.h>

namespace odessa
{
    c_memory::c_memory( const std::string &name ) noexcept
    {
        while ( !m_process )
        {
            const auto snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
            if ( snapshot == INVALID_HANDLE_VALUE )
            {
                Sleep( 500 );
                continue;
            }

            PROCESSENTRY32 proc { .dwSize = sizeof( PROCESSENTRY32 ) };

            if ( Process32First( snapshot, &proc ) )
            {
                do
                {
                    if ( name == proc.szExeFile )
                    {
                        m_pid     = static_cast< std::int32_t >( proc.th32ProcessID );
                        m_process = OpenProcess( PROCESS_ALL_ACCESS, FALSE, proc.th32ProcessID );
                        break;
                    }
                } while ( Process32Next( snapshot, &proc ) );
            }

            CloseHandle( snapshot );

            if ( !m_process )
                Sleep( 500 );
        }
    }

    c_memory::~c_memory( ) noexcept
    {
        if ( m_process )
        {
            CloseHandle( m_process );
            m_process = nullptr;
        }

        m_pid = 0;
    }

    std::unique_ptr< module_t > c_memory::module( const std::string &name ) const noexcept
    {
        if ( !m_process or m_pid == 0 )
            return nullptr;

        const auto snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pid );
        if ( snapshot == INVALID_HANDLE_VALUE )
            return nullptr;

        MODULEENTRY32 mod { .dwSize = sizeof( MODULEENTRY32 ) };

        std::unique_ptr< module_t > result;

        if ( Module32First( snapshot, &mod ) )
        {
            do
            {
                if ( name == mod.szModule )
                {
                    result = std::make_unique< module_t >( );

                    result->base = reinterpret_cast< std::uint64_t >( mod.modBaseAddr );
                    result->size = mod.modBaseSize;
                    result->name = mod.szModule;
                    result->path = mod.szExePath;
                    break;
                }
            } while ( Module32Next( snapshot, &mod ) );
        }

        CloseHandle( snapshot );
        return result;
    }

    std::uint64_t c_memory::find( const std::vector< std::uint8_t > &pattern ) const noexcept
    {
        const auto mod = module( constants::client_name );
        if ( !mod or pattern.empty( ) )
            return 0;

        std::uint64_t start = mod->base;
        std::uint64_t end   = mod->base + mod->size;

        MEMORY_BASIC_INFORMATION mbi { };

        while ( start < end )
        {
            if ( VirtualQueryEx( m_process, reinterpret_cast< void * >( start ), &mbi, sizeof( mbi ) ) == sizeof( mbi ) )
            {
                bool readable = ( mbi.State == MEM_COMMIT )
                            and ( ( mbi.Protect & PAGE_READONLY ) or ( mbi.Protect & PAGE_READWRITE ) or ( mbi.Protect & PAGE_WRITECOPY )
                                  or ( mbi.Protect & PAGE_EXECUTE_READ ) or ( mbi.Protect & PAGE_EXECUTE_READWRITE ) );

                if ( readable )
                {
                    auto region_size = mbi.RegionSize;
                    if ( start + region_size > end )
                        region_size = end - start;

                    auto buffer = read( start, region_size );
                    if ( buffer.empty( ) )
                    {
                        start += region_size;
                        continue;
                    }

                    for ( std::size_t idx = 0; idx + pattern.size( ) <= buffer.size( ); ++idx )
                    {
                        bool match = true;

                        for ( std::size_t pat_idx = 0; pat_idx < pattern.size( ); ++pat_idx )
                        {
                            if ( pattern[ pat_idx ] != 0xcc and buffer[ idx + pat_idx ] != pattern[ pat_idx ] )
                            {
                                match = false;
                                break;
                            }
                        }

                        if ( match )
                            return start + idx;
                    }
                }
                start += mbi.RegionSize;
            }
            else
                start += 0x1000;
        }

        return 0;
    }

    std::vector< std::uint64_t > c_memory::find_all( const std::vector< std::uint8_t > &pattern ) const noexcept
    {
        std::vector< std::uint64_t > results;

        const auto mod = module( constants::client_name );
        if ( !mod or pattern.empty( ) )
            return results;

        std::uint64_t start = mod->base;
        std::uint64_t end   = mod->base + mod->size;

        MEMORY_BASIC_INFORMATION mbi { };

        while ( start < end )
        {
            if ( VirtualQueryEx( m_process, reinterpret_cast< void * >( start ), &mbi, sizeof( mbi ) ) == sizeof( mbi ) )
            {
                bool readable = ( mbi.State == MEM_COMMIT )
                            and ( ( mbi.Protect & PAGE_READONLY ) or ( mbi.Protect & PAGE_READWRITE ) or ( mbi.Protect & PAGE_WRITECOPY )
                                  or ( mbi.Protect & PAGE_EXECUTE_READ ) or ( mbi.Protect & PAGE_EXECUTE_READWRITE ) );

                if ( readable )
                {
                    auto region_size = mbi.RegionSize;
                    if ( start + region_size > end )
                        region_size = end - start;

                    auto buffer = read( start, region_size );
                    if ( buffer.empty( ) )
                    {
                        start += region_size;
                        continue;
                    }

                    for ( std::size_t idx = 0; idx + pattern.size( ) <= buffer.size( ); ++idx )
                    {
                        bool match = true;

                        for ( std::size_t pat_idx = 0; pat_idx < pattern.size( ); ++pat_idx )
                        {
                            if ( pattern[ pat_idx ] != 0xcc and buffer[ idx + pat_idx ] != pattern[ pat_idx ] )
                            {
                                match = false;
                                break;
                            }
                        }

                        if ( match )
                            results.push_back( start + idx );
                    }
                }
                start += mbi.RegionSize;
            }
            else
                start += 0x1000;
        }

        return results;
    }

    std::uint64_t c_memory::rebase( const std::uint64_t address, e_rebase_type rebase_type ) const noexcept
    {
        const auto mod = module( constants::client_name );
        if ( !mod )
            return 0;

        switch ( rebase_type )
        {
            case e_rebase_type::sub :
                return address - mod->base;
            case e_rebase_type::add :
                return mod->base + address;
            default :
                return 0;
        }
    }
} // namespace odessa