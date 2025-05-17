#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    sampler2D diffuseTex;
    sampler2D specularTex;
};

// In/Out
in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

// Uniforms
uniform Material material;
uniform vec3 lightPos0;
uniform vec3 cameraPos;


void main() {
    vec3 lightDir = normalize(lightPos0 - vs_position);


    // Ambient lighting
    vec4 ambientFinal = vec4(material.ambient, 1.0f);


    // Diffused lighting
    float diffuse = max(dot(lightDir, normalize(vs_normal)), 0.0);
    vec4 diffuseFinal = vec4((material.diffuse * diffuse), 1.0f);


    // Specular lighting
    float specularFocalPoint = 30.0f;

    vec3 reflectDir = reflect(-lightDir, normalize(vs_normal));
    vec3 viewDir = normalize(cameraPos - vs_position);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), specularFocalPoint);
    vec4 specularFinal = vec4((material.specular * specular), 1.0f);


    fs_color = texture(material.diffuseTex, vs_texcoord) * vec4(vs_color, 1.0f) * (ambientFinal + diffuseFinal + specularFinal);
}
