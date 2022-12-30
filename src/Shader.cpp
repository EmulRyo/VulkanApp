#include <array>
#include <stdexcept>
#include <fstream>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Shader.h"

Shader::Shader(Device& device, int maxFramesInFlight, std::vector<Binding> bindings, const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename):
    m_device(device),
    m_maxFramesInFlight(maxFramesInFlight),
    m_bindings(bindings)
{
    m_descriptorSetLayout = CreateDescriptorSetLayout(bindings);
    m_stages = CreateStages(vertexShaderFilename, fragmentShaderFilename);
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

VkDescriptorSetLayout Shader::CreateDescriptorSetLayout(std::vector<Binding>& bindings) {

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (int i = 0; i < bindings.size(); i++) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = i;
        layoutBinding.descriptorCount = 1;

        if (bindings[i].Stage == StageSelection::Vertex)
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        else if (bindings[i].Stage == StageSelection::Fragment)
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        else if (bindings[i].Stage == StageSelection::All)
            layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

        if (bindings[i].Type == DescriptorType::Uniform) {
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        else if (bindings[i].Type == DescriptorType::Sampler) {
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.pImmutableSamplers = nullptr; // Optional
        }
        layoutBindings.push_back(layoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    VkDescriptorSetLayout layout;

    if (m_device.CreateDescriptorSetLayout(&layoutInfo, &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return layout;
}

std::vector<VkPipelineShaderStageCreateInfo> Shader::CreateStages(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
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

    return { vertShaderStageInfo, fragShaderStageInfo };
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
    for (int i = 0; i < m_bindings.size(); i++) {
        if (m_bindings[i].Type == DescriptorType::Uniform) {
            VkDeviceSize bufferSize = m_bindings[i].UniformSize;

            m_bindings[i]._Offset = (uint32_t)m_uniformBuffers.size();
            m_uniformBuffers.resize(m_uniformBuffers.size() + m_maxFramesInFlight);
            m_uniformBuffersMemory.resize(m_uniformBuffersMemory.size() + m_maxFramesInFlight);

            for (size_t j = 0; j < m_maxFramesInFlight; j++) {
                m_device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[m_bindings[i]._Offset + j], m_uniformBuffersMemory[m_bindings[i]._Offset + j]);
            }
        }
    }
}

void Shader::CreateDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes{};
    poolSizes.resize(m_bindings.size());
    for (int i = 0; i < m_bindings.size(); i++) {
        if (m_bindings[i].Type == DescriptorType::Uniform)
            poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        else if (m_bindings[i].Type == DescriptorType::Sampler)
            poolSizes[i].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[i].descriptorCount = static_cast<uint32_t>(m_maxFramesInFlight);
    }

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
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(m_bindings.size());
        for (int j = 0; j < m_bindings.size(); j++) {
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstSet = m_descriptorSets[i];
            descriptorWrites[j].dstBinding = j;
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorCount = 1;
            if (m_bindings[j].Type == DescriptorType::Uniform) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = m_uniformBuffers[m_bindings[j]._Offset + i];
                bufferInfo.offset = 0;
                bufferInfo.range = m_bindings[j].UniformSize;

                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[j].pBufferInfo = &bufferInfo;
            }
            else if (m_bindings[j].Type == DescriptorType::Sampler) {
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].pImageInfo = &imageInfo;
            }
        }

        m_device.UpdateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data());
    }
}

void Shader::UpdateUniformBuffer(uint32_t bindingID, uint32_t currentImage, void *data) {
    if (m_bindings[bindingID].Type != DescriptorType::Uniform)
        throw std::runtime_error("the binding is not a uniform buffer!");

    uint32_t offset = m_bindings[bindingID]._Offset;

    void* dst;
    m_device.MapMemory(m_uniformBuffersMemory[offset + currentImage], 0, m_bindings[bindingID].UniformSize, 0, &dst);
    memcpy(dst, data, m_bindings[bindingID].UniformSize);
    m_device.UnmapMemory(m_uniformBuffersMemory[offset + currentImage]);
}
