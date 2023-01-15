#include <array>
#include <stdexcept>
#include <fstream>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "DescriptorSet.h"

VkDescriptorSetLayout DescriptorSet::CreateLayout(Device& device, std::vector<Binding> bindings) {

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (int i = 0; i < bindings.size(); i++) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = i;
        layoutBinding.descriptorCount = 1;

        if (bindings[i].Stage == Shader::StageSelection::Vertex)
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        else if (bindings[i].Stage == Shader::StageSelection::Fragment)
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        else if (bindings[i].Stage == Shader::StageSelection::All)
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

    if (device.CreateDescriptorSetLayout(&layoutInfo, &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return layout;
}

VkDescriptorPool DescriptorSet::CreateDescriptorPool(Device &device) {
    std::vector<VkDescriptorPoolSize> poolSizes {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 20;

    VkDescriptorPool descriptorPool;
    if (device.CreateDescriptorPool(&poolInfo, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return descriptorPool;
}

VkDescriptorSet DescriptorSet::AllocateDescriptorSet(Device& device, VkDescriptorPool pool, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet descriptorSet;
    if (device.AllocateDescriptorSets(&allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return descriptorSet;
}

std::vector<VkDescriptorSet> DescriptorSet::AllocateDescriptorSets(Device &device, VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t count) {
    std::vector<VkDescriptorSetLayout> layouts(count, layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorSets(count);
    if (device.AllocateDescriptorSets(&allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return descriptorSets;
}

void DescriptorSet::UpdateUniformDescriptorSets(Device &device, std::vector<VkDescriptorSet> &descSets, uint32_t bindingID, VkBuffer &buffer, VkDeviceSize size) {
    std::vector<VkWriteDescriptorSet> descWrites(descSets.size());
    for (int i = 0; i < descSets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = size * i;
        bufferInfo.range = size;

        descWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrites[i].dstSet = descSets[i];
        descWrites[i].dstBinding = bindingID;
        descWrites[i].descriptorCount = 1;
        descWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrites[i].pBufferInfo = &bufferInfo;
    }
    device.UpdateDescriptorSets(static_cast<uint32_t>(descWrites.size()), descWrites.data());
}

void DescriptorSet::UpdateSamplerDescriptorSet(Device& device, VkDescriptorSet descSet, uint32_t bindingID, VkDescriptorImageInfo &imageInfo) {
    VkWriteDescriptorSet descWrite{};
    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite.dstSet = descSet;
    descWrite.dstBinding = bindingID;
    descWrite.descriptorCount = 1;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite.pImageInfo = &imageInfo;
    
    device.UpdateDescriptorSets(1, &descWrite);
}

void DescriptorSet::UpdateUniformBuffer(Device& device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, void* data) {
    void* dst;
    device.MapMemory(memory, offset, size, 0, &dst);
    memcpy(dst, data, size);
    device.UnmapMemory(memory);
}
