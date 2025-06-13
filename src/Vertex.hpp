#pragma once

#include <glm/glm.hpp>
#include <cstdint> // Required for int8_t and uint8_t

// Represents a single vertex with position, color, and normal attributes.
// Color and normal are packed into smaller types to save memory.
struct Vertex
{
    glm::vec3 position; // 12 bytes
    uint8_t color[4];   // 4 bytes (RGBA)
    int8_t normal[3];    // 3 bytes (XYZ)
    // Note: Due to memory alignment, the compiler will likely add 1 byte of padding
    // here to make the total struct size a multiple of 4 (20 bytes total).
};
