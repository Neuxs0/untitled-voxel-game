#pragma once

#include <glm/glm.hpp>

// Represents a single vertex with position, color, and normal attributes.
struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
};
