#include <array>
#include <stdexcept>
#include <fstream>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Shader.h"

Shader::Shader(Device& device, int maxFramesInFlight, size_t sizeUniform, const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename):
    m_device(device),
    m_maxFramesInFlight(maxFramesInFlight),
    m_sizeUniform(sizeUniform)
{
    CreateDescriptorSetLayout();
    CreateStages(vertexShaderFilename, fragmentShaderFilename);
}

Shader::~Shader() {
    m_device.DestroyShaderModule(m_fragShaderModule);
    m_device.DestroyShaderModule(m_vertShaderModule);
    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        m_device.DestroyBuffer(m_uniformBuffers[i]);
        m_device.FreeMemory(m_uniformBuffersMemory[i]);
    }
    m_device.DestroyDescriptorPool(m_descriptorPool);
    m_device.DestroyDescriptorSetLayout(m_descriptorSetLayout);
}

void Shader::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (m_device.CreateDescriptorSetLayout(&layoutInfo, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Shader::CreateStages(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
    auto vertShaderCode = ReadFile(vertexShaderFilename);
    auto fragShaderCode = ReadFile(fragmentShaderFilename);

    m_vertShaderModule = m_device.CreateShaderModule(vertShaderCode);
    m_fragShaderModule = m_device.CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragShaderModule;
    fragShaderStageInfo.pName = "main";

    m_stages = { vertShaderStageInfo, fragShaderStageInfo };
}

std::vector<char> Shader::ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void Shader::CreateUniformBuffers() {
    VkDeviceSize bufferSize = m_sizeUniform;

    m_uniformBuffers.resize(m_maxFramesInFlight);
    m_uniformBuffersMemory.resize(m_maxFramesInFlight);

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        m_device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
    }
}

void Shader::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_maxFramesInFlight);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(m_maxFramesInFlight);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_maxFramesInFlight);

    if (m_device.CreateDescriptorPool(&poolInfo, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Shader::CreateDescriptorSets(const VkDescriptorImageInfo& imageInfo) {
    std::vector<VkDescriptorSetLayout> layouts(m_maxFramesInFlight, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(m_maxFramesInFlight);
    if (m_device.AllocateDescriptorSets(&allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = m_sizeUniform;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        m_device.UpdateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data());
    }
}

void Shader::UpdateUniformBuffer(uint32_t currentImage, void *data) {
    void* dst;
    m_device.MapMemory(m_uniformBuffersMemory[currentImage], 0, m_sizeUniform, 0, &dst);
    memcpy(dst, data, m_sizeUniform);
    m_device.UnmapMemory(m_uniformBuffersMemory[currentImage]);
}
