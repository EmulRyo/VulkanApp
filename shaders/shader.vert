#version 450 

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec3 viewPos;
} global;

layout( push_constant ) uniform PushConstants {
	mat4 model;
	mat3x4 normal;
} pushConsts;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragTexCoord;

void main() {
    fragPosition = vec3(pushConsts.model * vec4(inPosition, 1.0)); // Posicion del vertice en world space
    fragNormal = mat3(pushConsts.normal) * inNormal;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    gl_Position = global.viewproj * pushConsts.model * vec4(inPosition, 1.0);
}