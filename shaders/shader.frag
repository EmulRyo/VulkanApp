#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec3 viewPos;
} global;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec3 diffuse;
    vec3 specular;
    float shininess;
    vec3 ambient;
} material;
layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 lightPos = normalize(vec3(1, 1, 1));
    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 normal = normalize(fragNormal);
    
    vec3 ambient = material.ambient;
    
    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseIntensity * material.diffuse;

    vec3 viewDir = normalize(global.viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularIntensity = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = specularIntensity * material.specular;  

    outColor = vec4(fragColor*(ambient + diffuse + specular), 1);

    outColor = texture(texSampler, fragTexCoord)*outColor;

    //outColor = vec4(specular, 1.0);
}