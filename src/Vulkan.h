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

void                    VulkanInit(Window& window, bool vSync);
Device*                 VulkanGetDevice();
Texture*                VulkanGetDummyTexture();
VkDescriptorPool        VulkanGetDescriptorPool();
VkDescriptorSetLayout   VulkanGetMaterialLayout();
void                    VulkanSetVSync(bool value);
void                    VulkanSetPipeline(int id);
void                    VulkanBeginDrawing();
void                    VulkanEndDrawing(bool recreateSwapchain);
void                    VulkanDraw(glm::mat4 matrix, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount, VkDescriptorSet materialDescSet);
void                    VulkanUpdateUniformBuffer(size_t bufferSize, void* data);
void                    VulkanWaitIdle();
void                    VulkanCleanupSwapChain();
void                    VulkanCleanup();

void                    VulkanImGuiInit();
void                    VulkanImGuiBeginDrawing();
void                    VulkanImGuiEndDrawing();
void                    VulkanImGuiCleanup();