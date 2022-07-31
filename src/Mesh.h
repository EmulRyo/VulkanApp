#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class Device;

struct Vertex {
    glm::vec3 pos;
    //glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const Vertex& other) const;
};

struct Texture {
    unsigned int id;
    std::string type;
};

class Mesh
{
public:
    Mesh(Device *device, std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures);
    Mesh(const Mesh& other);
    ~Mesh();
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets);

private:
    Device* m_device;
    // mesh data
    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<Texture>  m_textures;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

private:
    void CreateVertexBuffer();
    void CreateIndexBuffer();

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

