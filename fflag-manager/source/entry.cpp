#include "native.hpp"

// local->misc
#include "constants.hpp"
#include "memory/memory.hpp"

// local->engine
#include "fflags/fflags.hpp"
#include "engine/engine.hpp"

std::int32_t main( )
{
    odessa::g_memory         = std::make_unique< odessa::c_memory >( odessa::constants::client_name );
    odessa::engine::g_fflags = std::make_unique< odessa::engine::c_fflags >( );

    odessa::engine::setup( );

    return EXIT_SUCCESS;
}