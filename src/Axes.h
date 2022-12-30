#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include "Mesh.h"
#include "Model.h"

class Device;
class Mesh;
struct Material;

class Axes: public Model
{
public:
    Axes(Device& device, float length, float thickness)
        : Model(device) 
    { 
        Load(length, thickness);
    }
private:

    void Load(float length, float thickness);
    Mesh* CreateMesh(std::vector<uint32_t>& indices, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color);
};

