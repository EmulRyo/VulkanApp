#pragma once

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include "assimp/types.h"

class Device;
class Texture;
struct aiMaterial;

class Material {
public:
    Material(Device& device);
    Material(Device& device, const aiMaterial* assimpMat, const std::string& directory, std::vector<Texture *>& textures);
    ~Material();

    enum class ShadingModel {
        Error, Flat, Gouraud, Phong, Blinn, Toon, OrenNayar, Minnaert, CookTorrance, Unlit, Fresnel, PBR
    };

    const std::string& GetName() const { return m_name; }
    ShadingModel GetShadingModel() const { return m_shadingModel; }
    const std::string& GetShadingModelName() const;
    const glm::vec3& GetDiffuseColor() const {return m_diffuseColor; }
    const glm::vec3& GetSpecularColor() const { return m_specularColor; }
    const glm::vec3& GetAmbientColor() const { return m_ambientColor; }
    const glm::vec3& GetEmissiveColor() const { return m_emissiveColor; }
    const float GetShininess() const { return m_shininess; }

    void SetName(const std::string& name) { m_name = name; }
    void SetShadingModel(int shadingModel) { m_shadingModel = (ShadingModel)shadingModel; }
    void SetDiffuseColor(const glm::vec3& color) { m_diffuseColor = color; }
    void SetSpecularColor(const glm::vec3& color) { m_specularColor = color; }
    void SetAmbientColor(const glm::vec3& color) { m_ambientColor = color; }
    void SetEmissiveColor(const glm::vec3& color) { m_emissiveColor = color; }
    void SetShininess(float shininess) { m_shininess = shininess; }

    void UpdateUniform();

    const Texture* GetDiffuseTexture() const { return m_diffuseTex; }
    const Texture* GetSpecularTexture() const { return m_specularTex; }
    void SetDiffuseTexture(Texture* texture);
    void SetSpecularTexture(Texture* texture);

    void Print(const std::string& prefix) const;

    VkDescriptorSet GetDescriptorSet() const { return m_materialDescSet; }

    static glm::vec3 ToGlm(const aiColor3D& color3D) { return glm::vec3(color3D.r, color3D.g, color3D.b); };

    static void PrintColor(const std::string& prefix, const glm::vec3& color) {
        spdlog::debug("{} = ({:.3f}, {:.3f}, {:.3f})", prefix, color.r, color.g, color.b);
    }

    static void PrintTexture(const std::string& prefix, const Texture* tex);

private:
    Device& m_device;
    std::string m_name;
    ShadingModel m_shadingModel;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;
    glm::vec3 m_ambientColor;
    glm::vec3 m_emissiveColor;
    float m_shininess;
    Texture* m_diffuseTex;
    Texture* m_specularTex;

    struct MaterialUBO {
        alignas(16) glm::vec3 diffuse;
        alignas(16) glm::vec3 specular;
        alignas(4)  float shininess;
        alignas(16) glm::vec3 ambient;
        alignas(16) glm::vec3 emissive;
    };

    VkDescriptorSet m_materialDescSet;
    VkBuffer m_materialBuffer;
    VkDeviceMemory m_materialMemory;

    void Init();
    void LoadFromAssimp(const aiMaterial* assimpMat, const std::string& directory, std::vector<Texture*>& textures);
};