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
    Model(Device& device) : m_device(device) {};
    Model(Device& device, std::vector<Material*> materials, std::vector<Mesh*> meshes);
    Model(Device& device, const std::string &path): m_device(device) { Load(path); }
    ~Model();
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet globalSet);

    static ComponentType GetTypeStatic() { return ComponentType::Model; }
    ComponentType GetType() { return GetTypeStatic(); }

    glm::vec3 GetBBoxMin() { return m_bboxMin; }
    glm::vec3 GetBBoxMax() { return m_bboxMax; }
    glm::vec3 GetSize() { return { m_bboxMax.x - m_bboxMin.x, m_bboxMax.y - m_bboxMin.y, m_bboxMax.z - m_bboxMin.z }; }

    Transform Transform;
protected:
    Device& m_device;
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
    void PrintColor(const std::string& prefix, const glm::vec3& color);
    void PrintTexture(const std::string& prefix, const Texture* tex);
};

