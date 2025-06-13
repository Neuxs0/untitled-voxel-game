#include "EmbeddedShaders.hpp"

// Linker symbols for embedded shaders
extern const char _binary_src_assets_shaders_core_vert_glsl_start[];
extern const char _binary_src_assets_shaders_core_vert_glsl_end[];
extern const char _binary_src_assets_shaders_core_frag_glsl_start[];
extern const char _binary_src_assets_shaders_core_frag_glsl_end[];
extern const char _binary_src_assets_shaders_terrain_gen_comp_glsl_start[];
extern const char _binary_src_assets_shaders_terrain_gen_comp_glsl_end[];

// Definitions
const std::string_view EmbeddedShaders::core_vert(
    _binary_src_assets_shaders_core_vert_glsl_start,
    (size_t)(_binary_src_assets_shaders_core_vert_glsl_end - _binary_src_assets_shaders_core_vert_glsl_start)
);

const std::string_view EmbeddedShaders::core_frag(
    _binary_src_assets_shaders_core_frag_glsl_start,
    (size_t)(_binary_src_assets_shaders_core_frag_glsl_end - _binary_src_assets_shaders_core_frag_glsl_start)
);

const std::string_view EmbeddedShaders::terrain_gen_comp(
    _binary_src_assets_shaders_terrain_gen_comp_glsl_start,
    (size_t)(_binary_src_assets_shaders_terrain_gen_comp_glsl_end - _binary_src_assets_shaders_terrain_gen_comp_glsl_start)
);
