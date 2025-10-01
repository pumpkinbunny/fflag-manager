#pragma once

#include "native.hpp"

// local->engine
#include "fflags/fflags.hpp"

namespace odessa::engine
{
    /**
     * @brief Sets up the engine and initializes FFlag system.
     *
     * This function reads the fflags.json configuration file and initializes
     * the FFlag management system for the target process.
     */
    void setup( );
} // namespace odessa::engine