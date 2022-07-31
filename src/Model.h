#pragma once

#include <vector>
#include <string>

#include "Mesh.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Device;

class Model
{
public:
    Model(Device *device, const std::string &path): m_device(device) { Load(path); }
    ~Model();
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets);
private:
    Device* m_device;
    // model data
    std::vector<Mesh *> m_meshes;
    std::string m_directory;

    void Load(const std::string &path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh *ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

