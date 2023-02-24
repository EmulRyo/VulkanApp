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
class Pipeline;
class RenderImage;
class Axes;
class Prism;

class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();

    void run();
    VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }
    VkDescriptorSetLayout GetMaterialLayout() { return m_materialLayout; }
    Texture* GetDummyTexture() { return m_dummyTexture; }

    static VulkanApp* GetInstance();

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    //const std::string MODEL_PATH = "models/shiba/1.fbx";
    //const std::string MODEL_PATH = "models/swamp-location/map_1.obj";
    const std::string MODEL_PATH = "models/backpack/backpack.obj";
    //const std::string MODEL_PATH = "models/pony-cartoon/Pony_cartoon.obj";
    //const std::string MODEL_PATH = "models/che/scene.gltf";
    //const std::string MODEL_PATH = "models/bmw.obj";

    const int MAX_FRAMES_IN_FLIGHT = 2;

    Window m_window;
    ValidationLayers m_validationLayers;
    VkInstance m_instance;
    Device* m_device;
    Swapchain* m_swapchain;

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_globalLayout, m_materialLayout;
    std::vector<VkDescriptorSet> m_globalSet;
    VkBuffer m_globalBuffer;
    VkDeviceMemory m_globalMemory;

    Shader* m_phongShader;
    Shader* m_unlitShader;

    VkRenderPass m_renderPass;
    Pipeline* m_phongPipeline;
    Pipeline* m_unlitPipeline;
    Pipeline* m_selectedPipeline;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool m_framebufferResized = false;
    uint32_t currentFrame = 0;

    std::vector<GameObject *> m_gameObjects;
    Texture* m_dummyTexture;

    RenderImage* m_color;
    RenderImage* m_depth;

    using ChronoTime = std::chrono::steady_clock::time_point;
    Camera m_cam;
    CameraController m_camController;
    ChronoTime m_lastTime;

    void FramebufferResizeCallback(int width, int height);
    void KeyCallback(int key, int scancode, int action, int mods);

    void initVulkan();

    std::vector<VkDescriptorSetLayoutBinding> GetGlobalBindings();
    std::vector<VkDescriptorSetLayoutBinding> GetMaterialBindings();

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

    GameObject* NewGameObject(const std::string name, std::string modelPath);

    void Update(float deltaTime);
    void Draw(float deltaTime);

    void cleanupSwapChain();
    void cleanup();
};
