#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 ambient = vec3(0.1);
    vec3 lightDir = normalize(vec3(-1, 0, 1));
    vec3 normal = normalize(fragNormal);
    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseIntensity * vec3(1);
    outColor = vec4(fragColor*(ambient + diffuse), 1);

    //outColor = texture(texSampler, fragTexCoord);
}