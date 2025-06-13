#pragma once

#include <glm/glm.hpp>
#include <cstdint>

// Represents a single vertex with position, normal, color, and texture coordinates.
// Aligned for performance.
struct Vertex
{
    // Local chunk coordinates (e.g., 0-16).
    glm::vec3 position;     // 12 bytes
    // Face normal vector.
    int8_t normal[3];       // 3 bytes
    // Explicit padding to align `atlasOffset`.
    uint8_t padding;        // 1 byte
    // Base UV coordinates of the texture tile in the atlas (Tx, Ty).
    glm::vec2 atlasOffset;  // 8 bytes
    // Surface coordinates for texture repetition (s, t), ranging e.g. from 0 to quad_width.
    glm::vec2 surfaceCoords;// 8 bytes
    // Standard RGBA color (can be used for tinting).
    uint8_t color[4];       // 4 bytes
    // Total: 12 + 3 + 1 + 8 + 8 + 4 = 36 bytes.
};
