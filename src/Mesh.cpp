#include "Device.h"

#include "Mesh.h"


Mesh::Mesh(Device* device, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures):
    m_device(device),
    m_vertices(vertices),
    m_indices(indices),
    m_textures(textures)
{

    CreateVertexBuffer();
    CreateIndexBuffer();
}

Mesh::Mesh(const Mesh& other) :
    Mesh(other.m_device, other.m_vertices, other.m_indices, other.m_textures)
{

}

Mesh::~Mesh() {
    
    vkDestroyBuffer(m_device->GetHandle(), m_indexBuffer, nullptr);
    vkFreeMemory(m_device->GetHandle(), m_indexBufferMemory, nullptr);
    vkDestroyBuffer(m_device->GetHandle(), m_vertexBuffer, nullptr);
    vkFreeMemory(m_device->GetHandle(), m_vertexBufferMemory, nullptr);
    
}

void Mesh::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const VkDescriptorSet *descriptorSets)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets, 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}

void Mesh::CreateVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device->GetHandle(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device->GetHandle(), stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device->GetHandle(), stagingBuffer, nullptr);
    vkFreeMemory(m_device->GetHandle(), stagingBufferMemory, nullptr);
}

void Mesh::CreateIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device->GetHandle(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device->GetHandle(), stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device->GetHandle(), stagingBuffer, nullptr);
    vkFreeMemory(m_device->GetHandle(), stagingBufferMemory, nullptr);
}

void Mesh::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device->GetHandle(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->GetHandle(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device->GetHandle(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_device->GetHandle(), buffer, bufferMemory, 0);
}

void Mesh::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device->GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device->GetHandle(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->GetGraphicsQueue());

    vkFreeCommandBuffers(m_device->GetHandle(), m_device->GetCommandPool(), 1, &commandBuffer);
}

uint32_t Mesh::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    m_device->GetMemoryProperties(&memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
