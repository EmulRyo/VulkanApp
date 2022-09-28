#pragma once
#include <vector>
#include <string>

class Device;

class Shader
{
public:
	enum class DescriptorType {
		Uniform,
		Sampler
	};

	enum class StageSelection {
		Vertex,
		Fragment,
		All
	};

	struct Binding {
		DescriptorType Type;
		StageSelection Stage;
		size_t UniformSize; // Only for uniforms, not used in samplers

		// Private
		uint32_t _Offset;
	};

	Shader(Device& device, int maxFramesInFlight, std::vector<Binding> bindings, const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
	~Shader();

	std::vector<VkPipelineShaderStageCreateInfo>& GetStages() { return m_stages; }
	VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_descriptorSetLayout; };
	VkDescriptorSet& GetDescriptorSet(uint32_t currentImage) { return m_descriptorSets[currentImage]; };

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets(const VkDescriptorImageInfo& imageInfo);
	void UpdateUniformBuffer(uint32_t bindingID, uint32_t currentImage, void* data);

private:
	Device& m_device;
	const int m_maxFramesInFlight;
	std::vector<Binding> m_bindings;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;
	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<VkPipelineShaderStageCreateInfo> m_stages;
	VkShaderModule m_vertShaderModule;
	VkShaderModule m_fragShaderModule;

	VkDescriptorSetLayout CreateDescriptorSetLayout(std::vector<Binding>& bindings);
	std::vector<char> ReadFile(const std::string& filename);
	std::vector<VkPipelineShaderStageCreateInfo> CreateStages(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
};

