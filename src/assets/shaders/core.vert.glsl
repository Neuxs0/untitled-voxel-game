#version 460 core

// Input vertex attributes
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;
layout (location = 2) in vec3 vertex_normal;

// Output variables for the fragment shader
out vec3 vs_position;
out vec4 vs_color;
out vec3 vs_normal;

// Uniforms
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main() {
    // Transform vertex position to world space
    vs_position = vec4(ModelMatrix * vec4(vertex_position, 1.0f)).xyz;
    vs_color = vertex_color;
    // Transform normal to world space
    vs_normal = normalize(mat3(transpose(inverse(ModelMatrix))) * vertex_normal);
    // Transform vertex position to clip space
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(vertex_position, 1.0f);
}
