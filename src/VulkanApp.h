#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "Window.h"
#include "Shader.h"
#include "ValidationLayers.h"
#include "Camera.h"
#include "CameraController.h"

class Device;
class Model;
class Texture;
class Swapchain;
class RenderImage;
class Prism;

class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();

    void run();

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const std::string MODEL_PATH = "models/shiba/1.fbx";
    //const std::string MODEL_PATH = "models/swamp-location/map_1.obj";
    //const std::string MODEL_PATH = "models/backpack/backpack.obj";
    //const std::string MODEL_PATH = "models/pony-cartoon/Pony_cartoon.obj";
    //const std::string MODEL_PATH = "models/che/scene.gltf";
    const std::string TEXTURE_PATH = "textures/viking_room.png";

    const int MAX_FRAMES_IN_FLIGHT = 2;

    Window m_window;
    ValidationLayers m_validationLayers;
    VkInstance m_instance;
    Device* m_device;
    Swapchain* m_swapchain;
    Shader* m_shader;

    VkRenderPass m_renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool m_framebufferResized = false;
    uint32_t currentFrame = 0;

    GameObject m_gameObject;
    Texture* m_texture;

    RenderImage* m_color;
    RenderImage* m_depth;

    using ChronoTime = std::chrono::steady_clock::time_point;
    Camera m_cam;
    CameraController m_camController;
    ChronoTime m_lastTime;
    Prism *m_axisR, * m_axisG, * m_axisB;

    void FramebufferResizeCallback(int width, int height);

    void initVulkan();

    std::vector<Shader::Binding> GetBindings();

    void checkExtensions();

    void createInstance();

    void recreateSwapChain();

    void createRenderPass();
    void createGraphicsPipeline();
    void CreateRenderImages();

    void createCommandBuffers();

    void createSyncObjects();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void updateUniformBuffer(uint32_t currentImage);

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();

    void Update(float deltaTime);
    void Draw(float deltaTime);

    void cleanupSwapChain();
    void cleanup();
};
