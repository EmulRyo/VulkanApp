#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Logger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <spdlog/spdlog.h>

#include "Mesh.h"
#include "Model.h"

class AssimpStream : public Assimp::LogStream {
public: void write(const char* message) { spdlog::error("{}", message); }
};

Model::~Model() {
    for (unsigned int i = 0; i < m_meshes.size(); i++)
        delete m_meshes[i];
    for (unsigned int i = 0; i < m_materials.size(); i++)
        delete m_materials[i];
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets)
{
    for (unsigned int i = 0; i < m_meshes.size(); i++)
        m_meshes[i]->Draw(commandBuffer, pipelineLayout, descriptorSets);
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
        aiProcess_ValidateDataStructure
        //aiProcess_GlobalScale
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        spdlog::error("ASSIMP: {}", importer.GetErrorString());
        return;
    }
    m_directory = path.substr(0, path.find_last_of('/'));

    ProcessMaterials(scene);
    ProcessNode(scene->mRootNode, scene);
    spdlog::info(
        "Model \"{}\":\n{} meshes, {} vertices, {} indices ({} triangles)",path, m_meshes.size(), m_numVertices, m_numIndices, m_numIndices / 3);
}

void Model::ProcessMaterials(const aiScene* scene) {
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        Material* material = new Material();

        aiMaterial* mat = scene->mMaterials[i];

        aiString name;
        mat->Get(AI_MATKEY_NAME, name);
        material->name = name.C_Str();

        int shadingModel = 0;
        mat->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

        aiColor3D vec3;

        mat->Get(AI_MATKEY_COLOR_DIFFUSE, vec3);
        material->diffuse = Material::ToGlm(vec3);
        mat->Get(AI_MATKEY_COLOR_AMBIENT, vec3);
        material->ambient = Material::ToGlm(vec3);
        mat->Get(AI_MATKEY_COLOR_SPECULAR, vec3);
        material->specular = Material::ToGlm(vec3);
        mat->Get(AI_MATKEY_COLOR_EMISSIVE, vec3);
        material->emissive = Material::ToGlm(vec3);

        std::string shadingModeNames[] = {
            "Error", "Flat", "Gouraud", "Phong", "Blinn", "Toon", "OrenNayar", "Minnaert", "CookTorrance", "Unlit", "Fresnel", "PBR"
        };

        spdlog::debug("Material {}: name={}, shadingModel={}", i, material->name, shadingModeNames[shadingModel]);
        spdlog::debug("\tdiffuse  = ({:.3f}, {:.3f}, {:.3f})", material->diffuse.r, material->diffuse.g, material->diffuse.b);
        spdlog::debug("\tambient  = ({:.3f}, {:.3f}, {:.3f})", material->ambient.r, material->ambient.g, material->ambient.b);
        spdlog::debug("\tspecular = ({:.3f}, {:.3f}, {:.3f})", material->specular.r, material->specular.g, material->specular.b);
        spdlog::debug("\temissive = ({:.3f}, {:.3f}, {:.3f})", material->emissive.r, material->emissive.g, material->emissive.b);

        aiString texPath;
        for (unsigned int i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++) {
            mat->GetTexture(aiTextureType_DIFFUSE, i, &texPath);
            spdlog::debug("\ttex diffuse = {}", texPath.C_Str());
        }
        for (unsigned int i = 0; i < mat->GetTextureCount(aiTextureType_SPECULAR); i++) {
            mat->GetTexture(aiTextureType_SPECULAR, i, &texPath);
            spdlog::debug("\ttex specular = {}", texPath.C_Str());
        }

        m_materials.push_back(material);
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
        Mesh* mesh = ProcessMesh(assimpMesh, scene);
        m_numVertices += mesh->GetNumVertices();
        m_numIndices += mesh->GetNumIndices();
        spdlog::debug(
            "Mesh {}: vertices: {}, indices: {}({} triangles), material: \"{}\"",
            m_meshes.size(), mesh->GetNumVertices(), mesh->GetNumIndices(), mesh->GetNumIndices()/3, mesh->GetMaterial()->name);
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

    return new Mesh(m_device, vertices, indices, material);
}
