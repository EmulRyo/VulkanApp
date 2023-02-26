#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 viewPos;
} global;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec3 diffuse;
    vec3 specular;
    float shininess;
    vec3 ambient;
} material;
layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // ambient
    vec3 ambient = texture(diffuseSampler, fragTexCoord).rgb * material.ambient;
        
    // diffuse
    vec3 diffuse = texture(diffuseSampler, fragTexCoord).rgb * material.diffuse;
        
    // specular
    vec3 specular = texture(specularSampler, fragTexCoord).rgb * material.specular;

    outColor = vec4(fragColor*(ambient + diffuse + specular), 1);
}