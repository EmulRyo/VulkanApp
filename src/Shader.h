#pragma once
#include <vector>
#include <string>

class Device;

class Shader
{
public:
	enum class StageSelection {
		Vertex,
		Fragment,
		All
	};

	Shader(Device& device, int maxFramesInFlight, const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
	~Shader();

	std::vector<VkPipelineShaderStageCreateInfo>& GetStages() { return m_stages; }

private:
	Device& m_device;
	const int m_maxFramesInFlight;
	std::vector<VkPipelineShaderStageCreateInfo> m_stages;
	VkShaderModule m_vertShaderModule;
	VkShaderModule m_fragShaderModule;

	std::vector<char> ReadFile(const std::string& filename);
	std::vector<VkPipelineShaderStageCreateInfo> CreateStages(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
};

