#version 460 core

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

uniform sampler2D texture0;
uniform vec3 lightPos0;
uniform vec3 cameraPos;

void main() {
    // Ambient lighting
    vec3 ambientLight = vec3(0.1f, 0.1f, 0.1f);


    // Diffused lighting
    vec3 posToLightDirVec = normalize(vs_position - lightPos0);
    vec3 diffuseColor = vec3(1.0f, 1.0f, 1.0f);
    float diffuse = clamp(dot(posToLightDirVec, vs_normal), 0, 1);
    vec3 diffuseFinal = diffuseColor * diffuse;


    // Specular lighting
    vec3 lightToPosDirVec = normalize(lightPos0 - vs_position);
    vec3 reflectDirVec = normalize(reflect(lightToPosDirVec, normalize(vs_normal)));
    vec3 posToViewDirVec = normalize(vs_position - cameraPos);
    float specular = pow(max(dot(posToViewDirVec, reflectDirVec), 0), 30);
    vec3 specularFinal = vec3(1.0f, 1.0f, 1.0f) * specular;


    // Attenuation



    // Final lighting


    fs_color = texture(texture0, vs_texcoord) * vec4(vs_color, 1.0f) * (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.0f) + vec4(specularFinal, 1.0f));
}
