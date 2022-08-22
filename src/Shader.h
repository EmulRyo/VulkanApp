#pragma once
#include <vector>
#include <string>

class Device;

class Shader
{
public:
	Shader(Device& device, int maxFramesInFlight, size_t sizeUniform, const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
	~Shader();

	std::vector<VkPipelineShaderStageCreateInfo>& GetStages() { return m_stages; }
	VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_descriptorSetLayout; };
	VkDescriptorSet& GetDescriptorSet(uint32_t currentImage) { return m_descriptorSets[currentImage]; };

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets(const VkDescriptorImageInfo& imageInfo);
	void UpdateUniformBuffer(uint32_t currentImage, void* data);

private:
	Device& m_device;
	const int m_maxFramesInFlight;
	size_t m_sizeUniform;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;
	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<VkPipelineShaderStageCreateInfo> m_stages;
	VkShaderModule m_vertShaderModule;
	VkShaderModule m_fragShaderModule;

	void CreateDescriptorSetLayout();
	std::vector<char> ReadFile(const std::string& filename);
	void CreateStages(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
};
