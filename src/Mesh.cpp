#include <array>

#include "Device.h"
#include "Texture.h"
#include "Material.h"
#include "Vulkan.h"

#include "Mesh.h"

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, color);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

Mesh::Mesh(
    const std::vector<Vertex>& vertices, 
    const std::vector<unsigned int>& indices, 
    Material *material,
    const glm::vec3& bboxMin,
    const glm::vec3& bboxMax)
:
    m_vertices(vertices),
    m_indices(indices),
    m_material(material),
    m_bboxMin(bboxMin),
    m_bboxMax(bboxMax)
{
    Device* device = VulkanGetDevice();
    CreateVertexBuffer(device);
    CreateIndexBuffer(device);
}

Mesh::Mesh(const Mesh& other) :
    Mesh(other.m_vertices, other.m_indices, other.m_material, other.m_bboxMin, other.m_bboxMin)
{

}

Mesh::~Mesh() {
    Device* device = VulkanGetDevice();
    device->DestroyBuffer(m_indexBuffer);
    device->FreeMemory(m_indexBufferMemory);
    device->DestroyBuffer(m_vertexBuffer);
    device->FreeMemory(m_vertexBufferMemory);
}

void Mesh::Draw(glm::mat4 matrix)
{
    VkDescriptorSet materialDescSet = VK_NULL_HANDLE;
    if (m_material != nullptr)
        materialDescSet = m_material->GetDescriptorSet();
    VulkanDraw(matrix, m_vertexBuffer, m_indexBuffer, m_indices.size(), materialDescSet);
}

void Mesh::CreateVertexBuffer(Device* device) {
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    device->MapMemory(stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    device->UnmapMemory(stagingBufferMemory);

    device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    device->CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    device->DestroyBuffer(stagingBuffer);
    device->FreeMemory(stagingBufferMemory);
}

void Mesh::CreateIndexBuffer(Device* device) {
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    device->MapMemory(stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    device->UnmapMemory(stagingBufferMemory);

    device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    device->CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    device->DestroyBuffer(stagingBuffer);
    device->FreeMemory(stagingBufferMemory);
}
