#include <spdlog/spdlog.h>

#include "Mesh.h"
#include "Prism.h"

void Prism::Load(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color) {
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
    Material* material = nullptr;
    Mesh* mesh = new Mesh(m_device, vertices, indices, material, { xMin, yMin, zMin }, { xMax, yMax, zMax });
    m_meshes.push_back(mesh);
}
