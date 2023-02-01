#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "assimp/types.h"

class Device;
class Texture;
class Material;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

class Mesh
{
public:
    Mesh(
        Device& device, 
        const std::vector<Vertex>& vertices, 
        const std::vector<uint32_t>& indices, 
        Material *material, 
        const glm::vec3& bboxMin, 
        const glm::vec3& bboxMax
    );
    Mesh(const Mesh& other);
    ~Mesh();
    size_t GetNumVertices() { return m_vertices.size(); };
    size_t GetNumIndices() { return m_indices.size(); };
    Material* GetMaterial() { return m_material; }
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet globalSet);

private:
    Device& m_device;
    // mesh data
    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_indices;
    Material *m_material;
    glm::vec3 m_bboxMin;
    glm::vec3 m_bboxMax;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

private:
    void CreateVertexBuffer();
    void CreateIndexBuffer();
};

