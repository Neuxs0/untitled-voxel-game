#version 460 core

// Material properties
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Input variables from the vertex shader
in vec3 vs_position;
in vec4 vs_color;
in vec3 vs_normal;

// Output color
out vec4 fs_color;

// Uniforms
uniform Material material;
uniform vec3 lightPos0;
uniform vec3 cameraPos;
uniform bool u_isWireframe;

void main() {
    if (u_isWireframe)
    {
        fs_color = vs_color;
    }
    else
    {
        // Calculate lighting
        vec3 lightDir = normalize(lightPos0 - vs_position);

        // Ambient lighting
        vec3 ambientFinal = material.ambient;

        // Diffuse lighting
        float diffuse = max(dot(lightDir, normalize(vs_normal)), 0.0);
        vec3 diffuseFinal = material.diffuse * diffuse;

        // Specular lighting
        float specularFocalPoint = 30.0f;
        vec3 reflectDir = reflect(-lightDir, normalize(vs_normal));
        vec3 viewDir = normalize(cameraPos - vs_position);
        float specular = pow(max(dot(viewDir, reflectDir), 0.0), specularFocalPoint);
        vec3 specularFinal = material.specular * specular;

        // Final color
        vec3 lighting = ambientFinal + diffuseFinal + specularFinal;
        fs_color = vec4(lighting * vs_color.rgb, vs_color.a);
    }
}
