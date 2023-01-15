#include <array>
#include <stdexcept>
#include <fstream>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Shader.h"

Shader::Shader(Device& device, int maxFramesInFlight, const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename):
    m_device(device),
    m_maxFramesInFlight(maxFramesInFlight)
{
    m_stages = CreateStages(vertexShaderFilename, fragmentShaderFilename);
}

Shader::~Shader() {
    m_device.DestroyShaderModule(m_fragShaderModule);
    m_device.DestroyShaderModule(m_vertShaderModule);
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
