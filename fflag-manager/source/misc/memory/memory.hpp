#pragma once

#include "native.hpp"

namespace odessa
{
    enum class e_rebase_type : std::uint8_t
    {
        sub = 0,
        add = 1
    };

    struct module_t
    {
        std::uint64_t base { 0 }; ///< Base address of the module
        std::uint32_t size { 0 }; ///< Size of the module in bytes

        std::string name { "" }; ///< Name of the module
        std::string path { "" }; ///< Full path to the module
    };

    class c_memory
    {
        HANDLE       m_process { nullptr }; ///< Handle to the target process
        std::int32_t m_pid { 0 };           ///< Process ID of the target process

      public:
        /**
         * @brief Retrieves the memory usage of a process by its name.
         *
         * @param name The name of the process whose memory usage is to be retrieved.
         */
        c_memory( const std::string &name ) noexcept;

        /**
         * @brief Destroys the c_memory object and releases any associated resources.
         */
        ~c_memory( ) noexcept;

        /**
         * @brief Retrieves a unique pointer to a module by its name.
         *
         * @param name The name of the module to retrieve.
         *
         * @return A std::unique_ptr to the requested module_t instance, or nullptr if the module is not found.
         */
        std::unique_ptr< module_t > module( const std::string &name ) const noexcept;

        /**
         * @brief Scans the specified module for a pattern and returns the first match. 0xCC in the pattern is a wildcard.
         *
         * @param pattern Byte pattern to search for (0xCC is a wildcard).
         *
         * @return Address of the first match, or 0 if not found.
         */
        std::uint64_t find( const std::vector< std::uint8_t > &pattern ) const noexcept;

        /**
         * @brief Scans the specified module for a pattern. 0xCC in the pattern is a wildcard.
         *
         * @param pattern Byte pattern to search for (0xCC is a wildcard).
         *
         * @return Vector of addresses where the pattern was found.
         */
        std::vector< std::uint64_t > find_all( const std::vector< std::uint8_t > &pattern ) const noexcept;

        /**
         * @brief Returns the rebased value of the given address.
         *
         * @param address The address to be rebased.
         * @param rebase_type The type of rebasing operation to perform (subtraction or addition). Default is subtraction.
         *
         * @return The rebased address as a 64-bit unsigned integer.
         */
        std::uint64_t rebase( const std::uint64_t address, e_rebase_type rebase_type = e_rebase_type::sub ) const noexcept;

        /**
         * @brief Reads memory from the target process at the specified address.
         *
         * @tparam type_t The type of data to read from memory.
         * @param address The memory address to read from.
         *
         * @return The value read from the specified memory address, or a default-constructed value if the read fails.
         */
        template < typename type_t >
        [[nodiscard]] type_t read( const std::uint64_t address ) const noexcept
        {
            type_t buffer { };
            ReadProcessMemory( m_process, reinterpret_cast< void * >( address ), &buffer, sizeof( type_t ), nullptr );
            return buffer;
        }

        /**
         * @brief Reads a buffer of bytes from the target process at the specified address.
         *
         * @param address The memory address to read from.
         * @param size The number of bytes to read.
         *
         * @return A vector containing the bytes read from memory. If the read fails, returns an empty vector.
         */
        [[nodiscard]] std::vector< std::uint8_t > read( const std::uint64_t address, std::size_t size ) const noexcept
        {
            if ( size == 0 )
                return { };

            std::vector< std::uint8_t > buffer( size );

            std::size_t bytes_read { 0 };

            if ( !ReadProcessMemory( m_process, reinterpret_cast< void * >( address ), buffer.data( ), size, &bytes_read ) )
                return { };

            if ( bytes_read < size )
                buffer.resize( bytes_read );

            return buffer;
        }

        /**
         * @brief Writes data to the target process's memory at the specified address.
         *
         * @tparam type_t The type of data to write to memory.
         * @param address The memory address to write to.
         * @param value The value to write to the specified memory address.
         *
         * @return True if the write operation was successful; otherwise, false.
         */
        template < typename type_t >
        [[nodiscard]] bool write( const std::uint64_t address, const type_t &value ) const noexcept
        {
            return WriteProcessMemory( m_process, reinterpret_cast< void * >( address ), &value, sizeof( type_t ), nullptr ) != 0;
        }

        /**
         * @brief Writes a value of the specified type to a given memory address in a process.
         *
         * @tparam type_t The type of the value to write.
         * @param address The memory address where the value will be written.
         * @param value The value to write to the specified address.
         * @param size The number of bytes to write.
         *
         * @return True if the write operation succeeds; otherwise, false.
         */
        template < typename type_t >
        [[nodiscard]] bool write( const std::uint64_t address, const type_t &value, std::size_t size ) const noexcept
        {
            return WriteProcessMemory( m_process, reinterpret_cast< void * >( address ), &value, size, nullptr ) != 0;
        }

        /**
         * @brief Returns the process handle associated with the object.
         *
         * @return The HANDLE representing the process.
         */
        [[nodiscard]] HANDLE handle( ) const noexcept
        {
            return m_process;
        }

        /**
         * @brief Returns the process identifier (PID) associated with the object.
         *
         * @return The process identifier (PID) as a 32-bit signed integer.
         */
        [[nodiscard]] std::int32_t pid( ) const noexcept
        {
            return m_pid;
        }
    };

    inline auto g_memory { std::unique_ptr< c_memory > {} };
} // namespace odessa