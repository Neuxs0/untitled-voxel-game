#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    sampler2D diffuseTex;
    sampler2D specularTex;
};

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

uniform Material material;
uniform vec3 lightPos0;
uniform vec3 cameraPos;

void main() {
    // Ambient lighting
    vec3 ambientLight = material.ambient;


    // Diffused lighting
    vec3 lightDir = normalize(lightPos0 - vs_position);
    float diffuse = max(dot(lightDir, normalize(vs_normal)), 0.0);
    vec3 diffuseFinal = material.diffuse * diffuse;


    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, normalize(vs_normal));
    vec3 viewDir = normalize(cameraPos - vs_position);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 30);
    vec3 specularFinal = material.specular * specular;


    fs_color = texture(material.diffuseTex, vs_texcoord) * vec4(vs_color, 1.0f) * (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.0f) + vec4(specularFinal, 1.0f));
}
