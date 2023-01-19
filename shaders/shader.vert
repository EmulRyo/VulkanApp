#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
} global;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

layout( push_constant ) uniform PushConstants {
	mat4 model;
	vec4 data;
} pushConsts;

void main() {
    fragColor = inColor;
    fragNormal = mat3(pushConsts.model) * inNormal;
    fragTexCoord = inTexCoord;
    gl_Position = global.viewproj * pushConsts.model * vec4(inPosition, 1.0);
}