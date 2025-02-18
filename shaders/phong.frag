#version 450

#define MAX_LIGHTS 8

struct Light {
    vec4 position;
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation; // x:constant, y:linear, z:quadratic
    vec4 cutOff; // x:inner, y:outter
};

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 viewPos;
    Light lights[MAX_LIGHTS];
    ivec4 numLights; // x:directional, y:point, z:spot
} global;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec3 diffuse;
    vec3 specular;
    float shininess;
    vec3 ambient;
    vec3 emissive;
} material;
layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 DirLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-vec3(light.direction));

    vec3 ambient = vec3(light.ambient * texture(diffuseSampler, fragTexCoord) * vec4(material.ambient, 0));
    
    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = vec3(light.diffuse * texture(diffuseSampler, fragTexCoord) * diffuseIntensity * vec4(material.diffuse, 0));

    vec3 reflectDir = reflect(-lightDir, normal);
    // pow: The result is undefined if x<0 or if x=0 and y≤0
    float specularIntensity = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 0.001));
    vec3 specular = vec3(light.specular * texture(specularSampler, fragTexCoord) * specularIntensity * vec4(material.specular, 0));

    return (ambient + diffuse + specular);
}

vec3 PointLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(vec3(light.position) - fragPosition);
    // diffuse shading
    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    // pow: The result is undefined if x<0 or if x=0 and y≤0
    float specularIntensity = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 0.001));
    // attenuation
    float distance    = length(vec3(light.position) - fragPosition);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + 
  			     light.attenuation.z * (distance * distance));    
    // combine results
    vec3 ambient = vec3(light.ambient * texture(diffuseSampler, fragTexCoord) * vec4(material.ambient, 0));
    vec3 diffuse = vec3(light.diffuse * texture(diffuseSampler, fragTexCoord) * diffuseIntensity * vec4(material.diffuse, 0));
    vec3 specular = vec3(light.specular * texture(specularSampler, fragTexCoord) * specularIntensity * vec4(material.specular, 0));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 SpotLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(vec3(light.position) - fragPosition);
    float theta = dot(lightDir, normalize(-vec3(light.direction)));
    float epsilon   = light.cutOff.x - light.cutOff.y; // Inner - Outter
    float intensity = clamp((theta - light.cutOff.y) / epsilon, 0.0, 1.0);
    
    // ambient
    vec3 ambient = vec3(light.ambient * texture(diffuseSampler, fragTexCoord) * vec4(material.ambient, 0));
        
    // diffuse 
    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = vec3(light.diffuse * texture(diffuseSampler, fragTexCoord) * diffuseIntensity * vec4(material.diffuse, 0));
        
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    // pow: The result is undefined if x<0 or if x=0 and y≤0
    float specularIntensity = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 0.001));
    vec3 specular = vec3(light.specular * texture(specularSampler, fragTexCoord) * specularIntensity * vec4(material.specular, 0));

    // attenuation
    float distance    = length(vec3(light.position) - fragPosition);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + 
  			        light.attenuation.z * (distance * distance));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    // we'll leave ambient unaffected so we always have a little light.
    diffuse  *= intensity;
    specular *= intensity;
            
    return (ambient + diffuse + specular);
}

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(vec3(global.viewPos) - fragPosition);
    
    vec3 color = vec3(0);
    int numLightsOffset = 0;
    for (int i=0; i<global.numLights.x; i++) {
        color += DirLight(global.lights[numLightsOffset+i], normal, viewDir);
    }
    
    numLightsOffset += global.numLights.x;
    for (int i=0; i<global.numLights.y; i++) {
        color += PointLight(global.lights[numLightsOffset+i], normal, viewDir);
    }
    numLightsOffset += global.numLights.y;
    for (int i=0; i<global.numLights.z; i++) {
        color += SpotLight(global.lights[numLightsOffset+i], normal, viewDir);
    }
    color = material.emissive + (fragColor * color);

    outColor = vec4(color, 1.0);
}