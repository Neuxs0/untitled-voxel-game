#version 460 core

// Input vertex attributes
layout (location = 0) in vec3 a_position;    // local position (0-16)
layout (location = 1) in vec4 a_color;       // vertex color for tinting
layout (location = 2) in vec3 a_normal;      // face normal
layout (location = 3) in vec2 a_atlasOffset; // UV of tile's origin in atlas (Tx, Ty)
layout (location = 4) in vec2 a_surfaceCoords; // Surface coords for repetition (s, t)

// Output variables for the fragment shader
out vec3 vs_position;
out vec4 vs_color;
out vec3 vs_normal;
out vec2 vs_atlasOffset;   // Pass through Tx, Ty
out vec2 vs_surfaceCoords; // Pass through s, t

// Uniforms
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main() {
    vec3 vertex_position = a_position;

    vs_position = vec4(ModelMatrix * vec4(vertex_position, 1.0f)).xyz;
    vs_color = a_color;
    vs_normal = normalize(mat3(transpose(inverse(ModelMatrix))) * a_normal);
    
    vs_atlasOffset = a_atlasOffset;
    vs_surfaceCoords = a_surfaceCoords;
    
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(vs_position, 1.0f);
}
