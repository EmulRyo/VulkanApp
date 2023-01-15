#pragma once
#include <vector>
#include <string>
#include "Shader.h"

class Device;

class DescriptorSet
{
public:
	enum class DescriptorType {
		Uniform,
		Sampler
	};

	struct Binding {
		DescriptorType Type;
		Shader::StageSelection Stage;
	};

	static VkDescriptorPool CreateDescriptorPool(Device& device);
	static VkDescriptorSetLayout CreateLayout(Device& device, std::vector<Binding> bindings);
	static VkDescriptorSet AllocateDescriptorSet(Device& device, VkDescriptorPool pool, VkDescriptorSetLayout layout);
	static std::vector<VkDescriptorSet> AllocateDescriptorSets(Device& device, VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t count);
	static void UpdateUniformDescriptorSets(Device& device, std::vector<VkDescriptorSet>& descSets, uint32_t bindingID, VkBuffer& buffer, VkDeviceSize size);
	static void UpdateSamplerDescriptorSet(Device& device, VkDescriptorSet descSet, uint32_t bindingID, VkDescriptorImageInfo& imageInfo);
	static void UpdateUniformBuffer(Device& device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, void* data);
};

