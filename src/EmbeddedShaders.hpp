#pragma once

#include <string_view>

// Provides access to shader sources that have been embedded into the executable.
struct EmbeddedShaders {
    static const std::string_view core_vert;
    static const std::string_view core_frag;
    static const std::string_view terrain_gen_comp;
};
