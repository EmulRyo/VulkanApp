#pragma once

#include <vector>
#include <string>
#include <limits>

#include "Components.h"
#include "Transform.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Device;
class Mesh;
class Material;

class Model: public Component
{
public:
    Model() {};
    Model(std::vector<Material*> materials, std::vector<Mesh*> meshes);
    Model(const std::string &path) { Load(path); }
    ~Model();
    void Draw(glm::mat4 matrix);

    static ComponentType GetTypeStatic() { return ComponentType::Model; }
    ComponentType GetType() { return GetTypeStatic(); }

    glm::vec3 GetBBoxMin() const { return m_bboxMin; }
    glm::vec3 GetBBoxMax() const { return m_bboxMax; }
    glm::vec3 GetSize() { return { m_bboxMax.x - m_bboxMin.x, m_bboxMax.y - m_bboxMin.y, m_bboxMax.z - m_bboxMin.z }; }

    Transform Transform;
protected:
    // model data
    std::vector<Material *> m_materials;
    std::vector<Texture*> m_textures;
    std::vector<Mesh *> m_meshes;
    std::string m_directory;
    size_t m_numVertices = 0;
    size_t m_numIndices = 0;
    glm::vec3 m_bboxMin = glm::vec3(0);
    glm::vec3 m_bboxMax = glm::vec3(0);

private:
    void Load(const std::string &path);
    void ProcessMaterials(const aiScene* scene, VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh *ProcessMesh(aiMesh* mesh, const aiScene* scene);
};

