#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "Window.h"
#include "ValidationLayers.h"

class Device;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const Vertex& other) const;
};
/*
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
}*/

class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();

    void run();

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const std::string MODEL_PATH = "models/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/viking_room.png";

    const int MAX_FRAMES_IN_FLIGHT = 2;

    Window m_window;
    ValidationLayers m_validationLayers;
    VkInstance m_instance;
    Device* m_device;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool m_framebufferResized = false;
    uint32_t currentFrame = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    uint32_t mipLevels;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    static std::vector<char> readFile(const std::string& filename);
    void FramebufferResizeCallback(int width, int height);

    void initVulkan();

    void checkExtensions();

    void createInstance();

    void recreateSwapChain();

    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void createFramebuffers();

    void createCommandPool();
    void createCommandBuffers();

    void createSyncObjects();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createTextureImage();
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    void createTextureImageView();
    void createTextureSampler();
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();

    void createUniformBuffers();

    void createDescriptorPool();
    void createDescriptorSets();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void updateUniformBuffer(uint32_t currentImage);

    void createColorResources();
    void createDepthResources();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);

    void mainLoop();
    void drawFrame();

    void cleanupSwapChain();
    void cleanup();
};
