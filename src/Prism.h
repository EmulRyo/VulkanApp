#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include "Model.h"

class Device;
class Mesh;

class Prism: public Model
{
public:
    Prism(Device& device, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color)
        : Model(device) 
    { 
        Load(xMin, xMax, yMin, yMax, zMin, zMax, color);
    }
private:

    void Load(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, glm::vec3 color);
};

