#pragma once

#include <vector>
#include <string>

#include "Components.h"
#include "Transform.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Device;
class Mesh;
struct Material;

class Model: public Component
{
public:
    Model(Device& device) : m_device(device) {};
    Model(Device& device, std::vector<Material*> materials, std::vector<Mesh*> meshes);
    Model(Device& device, const std::string &path): m_device(device) { Load(path); }
    ~Model();
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets);

    static ComponentType GetTypeStatic() { return ComponentType::Model; }
    ComponentType GetType() { return GetTypeStatic(); }

    Transform Transform;
protected:
    Device& m_device;
    // model data
    std::vector<Material *> m_materials;
    std::vector<Mesh *> m_meshes;
    std::string m_directory;
    size_t m_numVertices = 0;
    size_t m_numIndices = 0;

private:
    void Load(const std::string &path);
    void ProcessMaterials(const aiScene* scene);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh *ProcessMesh(aiMesh* mesh, const aiScene* scene);
    //std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

