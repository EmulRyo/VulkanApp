#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>

class Device;
class Mesh;
struct Material;

class Prism
{
public:
    Prism(Device& device, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color): 
        m_device(device) 
    { 
        Load(xMin, xMax, yMin, yMax, zMin, zMax, color);
    }
    ~Prism();
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets);
private:
    Device& m_device;
    // model data
    std::vector<Mesh *> m_meshes;
    std::vector<Material *> m_materials;
    size_t m_numVertices = 0;
    size_t m_numIndices = 0;

    void Load(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color);
};

