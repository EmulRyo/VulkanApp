#pragma warning(push, 0) // Ocultar warnings de las librerias

#include "assimp/material.h"

#pragma warning(pop)

#include "Device.h"
#include "Texture.h"
#include "Material.h"

#include "Vulkan.h"

Material::Material() :
	m_name("No name"),
	m_diffuseColor(glm::vec3(1.0f)),
	m_specularColor(glm::vec3(0.0f)),
	m_ambientColor(glm::vec3(0.1f)),
	m_emissiveColor(glm::vec3(0.0f)),
	m_shininess(32.0f),
	m_diffuseTex(nullptr),
	m_specularTex(nullptr),
	m_materialDescSet(VK_NULL_HANDLE)
{
	Init();
}

Material::Material(const aiMaterial* assimpMat, const std::string& directory, std::vector<Texture *>& textures) :
	m_name("No name"),
	m_diffuseColor(glm::vec3(1.0f)),
	m_specularColor(glm::vec3(0.0f)),
	m_ambientColor(glm::vec3(0.1f)),
	m_emissiveColor(glm::vec3(0.0f)),
	m_shininess(32.0f),
	m_diffuseTex(nullptr),
	m_specularTex(nullptr),
	m_materialDescSet(VK_NULL_HANDLE)
{
	Init();

	LoadFromAssimp(assimpMat, directory, textures);
}

Material::~Material() {
	Device* device = VulkanGetDevice();
	device->DestroyBuffer(m_materialBuffer);
	device->FreeMemory(m_materialMemory);
}

void Material::Init() {
	VkDescriptorPool pool = VulkanGetDescriptorPool();
	VkDescriptorSetLayout layout = VulkanGetMaterialLayout();

	Device* device = VulkanGetDevice();

	device->CreateBuffer(
		sizeof(MaterialUBO),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_materialBuffer,
		m_materialMemory);

	m_materialDescSet = device->AllocateDescriptorSet(pool, layout);
	device->UpdateUniformDescriptorSet(m_materialDescSet, 0, m_materialBuffer, sizeof(MaterialUBO));

	SetDiffuseTexture(VulkanGetDummyTexture());
	SetSpecularTexture(VulkanGetDummyTexture());
}

void Material::LoadFromAssimp(const aiMaterial* assimpMat, const std::string& directory, std::vector<Texture*>& textures) {
	aiString name;
	assimpMat->Get(AI_MATKEY_NAME, name);
	SetName(name.C_Str());

	int shadingModel = 0;
	assimpMat->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
	SetShadingModel(shadingModel);

	float shininess = 0;
	assimpMat->Get(AI_MATKEY_SHININESS, shininess);
	SetShininess(shininess);

	aiColor3D vec3;

	assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, vec3);
	SetDiffuseColor(ToGlm(vec3));
	assimpMat->Get(AI_MATKEY_COLOR_AMBIENT, vec3);
	SetAmbientColor(ToGlm(vec3));
	assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, vec3);
	SetSpecularColor(ToGlm(vec3));
	assimpMat->Get(AI_MATKEY_COLOR_EMISSIVE, vec3);
	SetEmissiveColor(ToGlm(vec3));

	aiString texPath;
	for (unsigned int i = 0; i < assimpMat->GetTextureCount(aiTextureType_DIFFUSE); i++) {
		assimpMat->GetTexture(aiTextureType_DIFFUSE, i, &texPath);
		Texture* tex = new Texture(directory + "/" + texPath.C_Str());
		SetDiffuseTexture(tex);
		textures.push_back(tex);
	}

	for (unsigned int i = 0; i < assimpMat->GetTextureCount(aiTextureType_SPECULAR); i++) {
		assimpMat->GetTexture(aiTextureType_SPECULAR, i, &texPath);
		Texture* tex = new Texture(directory + "/" + texPath.C_Str());
		SetSpecularTexture(tex);
		textures.push_back(tex);
	}

	UpdateUniform();
}

const std::string& Material::GetShadingModelName() const {
	static std::string names[] = {
		"Error", "Flat", "Gouraud", "Phong", "Blinn", "Toon", "OrenNayar", "Minnaert", "CookTorrance", "Unlit", "Fresnel", "PBR"
	};
	return names[(int)m_shadingModel];
}

void Material::SetDiffuseTexture(Texture* texture) {
	m_diffuseTex = texture;
	VkDescriptorImageInfo imgInfo = texture->GetDescriptorImageInfo();
	VulkanGetDevice()->UpdateSamplerDescriptorSet(m_materialDescSet, 1, imgInfo);
}

void Material::SetSpecularTexture(Texture* texture) {
	m_specularTex = texture;
	VkDescriptorImageInfo imgInfo = texture->GetDescriptorImageInfo();
	VulkanGetDevice()->UpdateSamplerDescriptorSet(m_materialDescSet, 2, imgInfo);
}

void Material::UpdateUniform() {
	MaterialUBO ubo{};
	ubo.diffuse = m_diffuseColor;
	ubo.specular = m_specularColor;
	ubo.ambient = m_ambientColor;
	ubo.shininess = m_shininess;
	ubo.emissive = m_emissiveColor;

	VulkanGetDevice()->UpdateUniformBuffer(m_materialMemory, 0, sizeof(MaterialUBO), &ubo);
}

void Material::Print(const std::string& prefix) const {
	spdlog::debug("{}name={}, shadingModel={}", prefix, GetName(), GetShadingModelName());
	PrintColor("\tdiffuse  ", GetDiffuseColor());
	PrintColor("\tambient  ", GetAmbientColor());
	PrintColor("\tspecular ", GetSpecularColor());
	PrintColor("\temissive ", GetEmissiveColor());
	spdlog::debug("\tshininess = {}", GetShininess());
	PrintTexture("\ttex.diffuse ", GetDiffuseTexture());
	PrintTexture("\ttex.specular", GetSpecularTexture());
}

void Material::PrintTexture(const std::string& prefix, const Texture* tex) {
	if (tex && !tex->GetFilename().empty())
		spdlog::debug("{} = {}, ({}x{}x{})", prefix, tex->GetFilename(), tex->GetWidth(), tex->GetHeight(), tex->GetChannels());
}
