#pragma once

#include "native.hpp"

namespace odessa::constants
{
    static const std::string client_name = "RobloxPlayerBeta.exe"; ///< Name of the target process.

    static const std::vector< std::uint8_t > pattern
        = { 0x48, 0x83, 0xec, 0x38, 0x48, 0x8b, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc, 0x4c, 0x8d, 0x05 }; ///< Pattern to scan for.
} // namespace odessa::constants