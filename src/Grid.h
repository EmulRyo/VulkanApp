#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include "Mesh.h"
#include "Model.h"

class Device;
class Mesh;
struct Material;

class Grid: public Model
{
public:
    Grid(Device& device, int slices, float spacing, float thickness, glm::vec3 color = {0.9f, 0.9f, 0.9f})
        : Model(device) 
    { 
        Load(slices, spacing, thickness, color);
    }
private:

    void Load(int slices, float spacing, float thickness, glm::vec3 color);
    Mesh* CreateMesh(std::vector<uint32_t>& indices, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color);
};

