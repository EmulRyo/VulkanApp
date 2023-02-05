#include <spdlog/spdlog.h>

#include "Texture.h"
#include "Material.h"
#include "Device.h"
#include "Grid.h"

void Grid::Load(int slices, float spacing, float thickness, glm::vec3 color) {
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

    float w2 = thickness / 2.0f;
    int numBars = slices + 1;
    float length = slices * spacing;
    float l2 = length / 2.0f;
    float offset = -l2;
    for (int i = 0; i <numBars; i++) {
        Mesh* mesh = CreateMesh(indices, offset-w2, offset+w2, -w2, +w2, -l2, +l2, color);
        m_meshes.push_back(mesh);
        offset += spacing;
    }
    offset = -l2;
    for (int i = 0; i < numBars; i++) {
        Mesh* mesh = CreateMesh(indices, -l2, l2, -w2, +w2, offset-w2, offset+w2, color);
        m_meshes.push_back(mesh);
        offset += spacing;
    }
}

Mesh* Grid::CreateMesh(std::vector<uint32_t> &indices, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color) {
    Material* material = new Material(m_device);
    material->SetAmbientColor({ 1.0f, 1.0f, 1.0f });
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
