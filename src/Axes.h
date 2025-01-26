#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include "Mesh.h"
#include "Model.h"

class Mesh;

class Axes: public Model
{
public:
    Axes(float length, float thickness)
    { 
        Load(length, thickness);
    }
private:

    void Load(float length, float thickness);
    Mesh* CreateMesh(std::vector<uint32_t>& indices, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color);
};

