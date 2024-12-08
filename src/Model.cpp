#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Logger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <spdlog/spdlog.h>

#include "Mesh.h"
#include "Texture.h"
#include "Device.h"
#include "Material.h"
#include "VulkanApp.h"
#include "Model.h"

class AssimpStream : public Assimp::LogStream {
public: void write(const char* message) { spdlog::error("{}", message); }
};

Model::Model(Device& device, std::vector<Material*> materials, std::vector<Mesh*> meshes)
    : m_device(device), m_materials(materials), m_meshes(meshes)
{
    m_bboxMin.x = m_bboxMin.y = m_bboxMin.z = std::numeric_limits<float>::max();
    m_bboxMax.x = m_bboxMax.y = m_bboxMax.z = std::numeric_limits<float>::min();
    for (int i = 0; i < m_meshes.size(); i++) {
        m_numIndices += m_meshes[i]->GetNumIndices();
        m_numVertices += m_meshes[i]->GetNumVertices();
    }
}

Model::~Model() {
    for (unsigned int i = 0; i < m_meshes.size(); i++)
        delete m_meshes[i];
    for (unsigned int i = 0; i < m_materials.size(); i++)
        delete m_materials[i];
    for (unsigned int i = 0; i < m_textures.size(); i++)
        delete m_textures[i];
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet globalSet)
{
    for (unsigned int i = 0; i < m_meshes.size(); i++)
        m_meshes[i]->Draw(commandBuffer, pipelineLayout, globalSet);
}

void Model::Load(const std::string& path) {
    // Select the kinds of messages you want to receive on this log stream
    const unsigned int severity = Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn;

    // Attaching it to the default logger
    Assimp::DefaultLogger::get()->attachStream(new AssimpStream, severity);

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_ValidateDataStructure |
        aiProcess_GenBoundingBoxes |
        aiProcess_PreTransformVertices
        //aiProcess_GlobalScale
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        spdlog::error("ASSIMP: {}", importer.GetErrorString());
        return;
    }
    m_directory = path.substr(0, path.find_last_of('/'));

    aiMatrix4x4 m = scene->mRootNode->mTransformation;

    if (!m.IsIdentity()) {
        aiVector3D scale, rotation, position;
        m.Decompose(scale, rotation, position);

        Transform.Scale = glm::vec3(scale.x, scale.y, scale.z);
        Transform.Rotation = glm::vec3(rotation.x, rotation.y, rotation.z);
        Transform.Translation = glm::vec3(position.x, position.y, position.z);

        spdlog::debug("Assimp rootNode:\n[{}, {}, {}, {}\n {}, {}, {}, {}\n {}, {}, {}, {}\n {}, {}, {}, {}]\n",
            m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4);

    }

    if (scene->mRootNode->mMetaData) {
        aiMetadata* metadata = scene->mRootNode->mMetaData;
        spdlog::debug("Assimp metadata:");
        for (unsigned int i = 0; i < metadata->mNumProperties; i++) {
            spdlog::debug("{}: (type: {})", metadata->mKeys[i].C_Str(), (int)(metadata->mValues[i].mType));
        }
    }

    VkDescriptorPool pool = VulkanApp::GetInstance()->GetDescriptorPool();
    VkDescriptorSetLayout layout = VulkanApp::GetInstance()->GetMaterialLayout();
    ProcessMaterials(scene, pool, layout);
    ProcessNode(scene->mRootNode, scene);
    spdlog::info("Model \"{}\":\n\t{} meshes\n\t{} vertices\n\t{} indices\n\t{} triangles",
        path, m_meshes.size(), m_numVertices, m_numIndices, m_numIndices / 3);
    for (int i = 0; i < m_meshes.size(); i++) {
        spdlog::debug("Mesh {}: verts={}, inds={}, tris={}, material={}", 
            i, m_meshes[i]->GetNumVertices(), m_meshes[i]->GetNumIndices(), m_meshes[i]->GetNumIndices() / 3, m_meshes[i]->GetMaterial()->GetName());
        spdlog::debug("\tbbox = min({}, {}, {}), max({}, {}, {})",
            m_meshes[i]->GetBBoxMin().x, m_meshes[i]->GetBBoxMin().y, m_meshes[i]->GetBBoxMin().z,
            m_meshes[i]->GetBBoxMax().x, m_meshes[i]->GetBBoxMax().y, m_meshes[i]->GetBBoxMax().z);
    }
    for (int i = 0; i < m_materials.size(); i++)
        m_materials[i]->Print("Material "+ std::to_string(i) + ": ");
}

void Model::ProcessMaterials(const aiScene* scene, VkDescriptorPool pool, VkDescriptorSetLayout layout) {
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        Material* material = new Material(m_device, scene->mMaterials[i], m_directory, m_textures);

        m_materials.push_back(material);
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    aiVector3D scale, rotation, position;
    aiMatrix4x4 m = node->mTransformation;
    /*
    node->mTransformation.Decompose(scale, rotation, position);
    glm::vec3 scale = glm::vec3(scale.x, scale.y, scale.z);
    glm::vec3 rotation = glm::vec3(rotation.x, rotation.y, rotation.z);
    glm::vec3 translation = glm::vec3(position.x, position.y, position.z);
    */

    if (!m.IsIdentity()) {
        spdlog::debug("Assimp node:\n[{}, {}, {}, {}\n {}, {}, {}, {}\n {}, {}, {}, {}\n {}, {}, {}, {}]\n",
            m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4);
    }

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
        Mesh* mesh = ProcessMesh(assimpMesh, scene);
        m_numVertices += mesh->GetNumVertices();
        m_numIndices += mesh->GetNumIndices();
        m_meshes.push_back(mesh);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh *Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material *material = nullptr;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.pos = vector;
        
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;


        if (mesh->HasVertexColors(0)) {
            vector.x = mesh->mColors[0][i].r;
            vector.y = mesh->mColors[0][i].g;
            vector.z = mesh->mColors[0][i].b;
            vertex.color = vector;
        }
        else
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
        }
        else
            vertex.texCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0)
        material = m_materials[mesh->mMaterialIndex];

    glm::vec3 bboxMin = { mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z };
    glm::vec3 bboxMax = { mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z };

    m_bboxMin.x = std::min(m_bboxMin.x, bboxMin.x);
    m_bboxMin.y = std::min(m_bboxMin.y, bboxMin.y);
    m_bboxMin.z = std::min(m_bboxMin.z, bboxMin.z);
    m_bboxMax.x = std::max(m_bboxMax.x, bboxMax.x);
    m_bboxMax.y = std::max(m_bboxMax.y, bboxMax.y);
    m_bboxMax.z = std::max(m_bboxMax.z, bboxMax.z);

    return new Mesh(m_device, vertices, indices, material, bboxMin, bboxMax);
}
