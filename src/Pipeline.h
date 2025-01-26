#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "Shader.h"
#include "Device.h"

class Swapchain;

class Pipeline {
public:
    Pipeline(
        Device& device,
        VkRenderPass renderPass,
        Swapchain* swapchain,
        Shader* shader,
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    Pipeline(const Pipeline& other);
    ~Pipeline();

    void Build();

    void SetShader(Shader* shader) { m_shader = shader; }
    void SetMSAA(VkSampleCountFlagBits msaa) { m_msaa = msaa; }
    void SetPushConstantsSize(uint32_t size) { m_pushConstantsSize = size; }
    void SetWireframeMode(bool wireframe) { m_polygonMode = wireframe ? VkPolygonMode::VK_POLYGON_MODE_LINE : VkPolygonMode::VK_POLYGON_MODE_FILL; }
    void SetCullMode(VkCullModeFlagBits cullMode) { m_cullMode = cullMode; }

    VkPipeline Get() { return m_pipeline; }
    VkPipelineLayout GetLayout() { return m_layout; }

private:
    void Cleanup();

    Device& m_device;
    VkPipelineLayout m_layout;
    VkPipeline m_pipeline;
    VkRenderPass m_renderPass;
    Swapchain* m_swapchain;
    Shader* m_shader;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    VkSampleCountFlagBits m_msaa;
    uint32_t m_pushConstantsSize;
    VkPolygonMode m_polygonMode;
    VkCullModeFlagBits m_cullMode;
};
