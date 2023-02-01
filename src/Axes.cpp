#include <spdlog/spdlog.h>

#include "VulkanApp.h"
#include "Texture.h"
#include "Material.h"
#include "Device.h"
#include "Axes.h"

void Axes::Load(float length, float thickness) {
    std::vector<uint32_t> indices{
        0, 5, 1,
        0, 4, 5,
        2, 4, 0,
        2, 6, 4,
        3, 7, 2,
        2, 7, 6,
        3, 1, 5,
        3, 5, 7,
        5, 4, 6,
        5, 6, 7,
        0, 1, 3,
        0, 3, 2
    };
    
    Mesh* meshR = CreateMesh(indices, 0.0f, length, -thickness / 2.0f, +thickness / 2.0f, -thickness / 2.0f, +thickness / 2.0f, { 1.0f, 0.0f, 0.0f });
    Mesh* meshG = CreateMesh(indices, -thickness / 2.0f, +thickness / 2.0f, 0.0f, length, -thickness / 2.0f, +thickness / 2.0f, { 0.0f, 1.0f, 0.0f });
    Mesh* meshB = CreateMesh(indices, -thickness / 2.0f, +thickness / 2.0f, -thickness / 2.0f, +thickness / 2.0f, 0.0f, length, { 0.0f, 0.0f, 1.0f });
    m_meshes.push_back(meshR);
    m_meshes.push_back(meshG);
    m_meshes.push_back(meshB);
}

Mesh* Axes::CreateMesh(std::vector<uint32_t> &indices, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color) {
    Material* material = new Material(m_device);
    material->UpdateUniform();
    m_materials.push_back(material);

    std::vector<Vertex> vertices{
        {{xMin, yMin, zMin}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMin, yMin, zMax}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMin, yMax, zMin}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMin, yMax, zMax}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMax, yMin, zMin}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMax, yMin, zMax}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMax, yMax, zMin}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
        {{xMax, yMax, zMax}, {0.0f, 0.0f, 0.0f}, color, {0.0f, 0.0f}},
    };

    return new Mesh(m_device, vertices, indices, material, { xMin, yMin, zMin }, { xMax, yMax, zMax });
}
