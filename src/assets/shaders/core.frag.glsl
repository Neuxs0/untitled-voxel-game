#version 460 core

// Material properties
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// Input variables from the vertex shader
in vec3 vs_position;
in vec4 vs_color;
in vec3 vs_normal;
in vec2 vs_atlasOffset;   // Tile origin in atlas (Tx, Ty) - EXACT TOP-LEFT
in vec2 vs_surfaceCoords; // Surface repetition coords (s, t)

// Output color
out vec4 fs_color;

// Uniforms
uniform Material material;
uniform vec3 lightPos0;
uniform vec3 cameraPos;
uniform bool u_isWireframe;
uniform sampler2D u_textureAtlas;
uniform bool u_isTransparentPass;
uniform vec2 u_atlasTileSize; // Normalized size of one tile in the atlas (TileWidthPx/AtlasWidthPx, TileHeightPx/AtlasHeightPx) - EXACT SIZE
uniform float u_texturePixelDimension; // e.g., 16.0 for a 16x16 source texture

void main() {
    if (u_isWireframe)
    {
        fs_color = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else
    {
        // final_atlas_uv will now map to the shrunken area within the correct tile in the atlas
        vec2 final_atlas_uv = vs_atlasOffset + fract(vs_surfaceCoords) * u_atlasTileSize;
        
        vec4 textureColor = texture(u_textureAtlas, final_atlas_uv);
        float outputAlpha;

        if (u_isTransparentPass) {
            if (textureColor.a < 0.01) { 
                discard;
            }
            outputAlpha = 0.70; 
        } else {
            if (textureColor.a < 0.01) {
                discard;
            }
            outputAlpha = 1.0; 
        }

        vec3 lightDir = normalize(lightPos0 - vs_position);
        vec3 norm = normalize(vs_normal);

        vec3 ambientFinal = material.ambient;
        float diffuseStrength = max(dot(lightDir, norm), 0.0);
        vec3 diffuseFinal = material.diffuse * diffuseStrength;

        vec3 reflectDir = reflect(-lightDir, norm);
        vec3 viewDir = normalize(cameraPos - vs_position);
        float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specularFinal = material.specular * specularStrength;

        vec3 lighting = ambientFinal + diffuseFinal + specularFinal;
        fs_color = vec4(lighting * textureColor.rgb * vs_color.rgb, outputAlpha);
    }
}
