#pragma once

#include <glm/glm.hpp>

constexpr auto MAX_FRAMES_IN_FLIGHT = 2;
constexpr auto MAX_LIGHTS = 8;

struct Light {
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 attenuation;  // x:constant, y:linear, z:quadratic
    glm::vec4 cutOff;       // x:inner, y:outter
};

struct GlobalUBO {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
    glm::vec4 viewPos;
    Light lights[MAX_LIGHTS];
    glm::ivec4 numLights;   // x:directional, y:point, z:spot
};

struct PushConstants {
    glm::mat4 model;
    glm::mat3x4 normal;     // normalMatrix (3x3). Para evitar problemas de alineacion se usa una de 3x4
};

class Texture;

class Vulkan {
public:
    static void                    Init(Window& window, bool vSync);
    static Device*                 GetDevice();
    static Texture*                GetDummyTexture();
    static VkDescriptorPool        GetDescriptorPool();
    static VkDescriptorSetLayout   GetMaterialLayout();
    static void                    SetVSync(bool value);
    static void                    SetPipeline(int id);
    static void                    BeginDrawing();
    static void                    EndDrawing();
    static void                    Draw(glm::mat4 matrix, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount, VkDescriptorSet materialDescSet);
    static void                    UpdateUniformBuffer(size_t bufferSize, void* data);
    static void                    WaitIdle();
    static void                    Cleanup();

    static void                    ImGuiInit();
    static void                    ImGuiBeginDrawing();
    static void                    ImGuiEndDrawing();
    static void                    ImGuiCleanup();
};