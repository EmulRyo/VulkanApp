#pragma warning(push, 0) // Ocultar warnings de las librerias

#include <filesystem>

#include "assimp/scene.h"
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

Material::Material(const aiScene* scene, const aiMaterial* assimpMat, const std::string& directory, std::vector<Texture *>& textures) :
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

	LoadFromAssimp(scene, assimpMat, directory, textures);
}

Material::~Material() {
	Device* device = Vulkan::GetDevice();
	device->DestroyBuffer(m_materialBuffer);
	device->FreeMemory(m_materialMemory);
}

void Material::Init() {
	VkDescriptorPool pool = Vulkan::GetDescriptorPool();
	VkDescriptorSetLayout layout = Vulkan::GetMaterialLayout();

	Device* device = Vulkan::GetDevice();

	device->CreateBuffer(
		sizeof(MaterialUBO),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_materialBuffer,
		m_materialMemory);

	m_materialDescSet = device->AllocateDescriptorSet(pool, layout);
	device->UpdateUniformDescriptorSet(m_materialDescSet, 0, m_materialBuffer, sizeof(MaterialUBO));

	SetDiffuseTexture(Vulkan::GetDummyTexture());
	SetSpecularTexture(Vulkan::GetDummyTexture());
}

void Material::LoadFromAssimp(const aiScene* scene, const aiMaterial* assimpMat, const std::string& directory, std::vector<Texture*>& textures) {
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

	aiReturn r = aiReturn_FAILURE;
	r = assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, vec3);
	SetDiffuseColor(r == aiReturn_SUCCESS ? ToGlm(vec3) : glm::vec3(0));
	r = assimpMat->Get(AI_MATKEY_COLOR_AMBIENT, vec3);
	SetAmbientColor(r == aiReturn_SUCCESS ? ToGlm(vec3) : glm::vec3(0));
	r = assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, vec3);
	SetSpecularColor(r == aiReturn_SUCCESS ? ToGlm(vec3) : glm::vec3(0));
	r = assimpMat->Get(AI_MATKEY_COLOR_EMISSIVE, vec3);
	SetEmissiveColor(r == aiReturn_SUCCESS ? ToGlm(vec3) : glm::vec3(0));
	
	Texture* diffuseTex  = GetTexture(scene, assimpMat, directory, aiTextureType_DIFFUSE, 0);
	Texture* specularTex = GetTexture(scene, assimpMat, directory, aiTextureType_SPECULAR, 0);
	if (diffuseTex) {
		SetDiffuseTexture(diffuseTex);
		textures.push_back(diffuseTex);
	}
	if (specularTex) {
		SetSpecularTexture(specularTex);
		textures.push_back(specularTex);
	}

	UpdateUniform();
}

Texture* Material::GetTexture(const aiScene* scene, const aiMaterial* assimpMat, const std::string& directory, aiTextureType type, unsigned int index) const {
	aiString texPath;
	aiReturn r = assimpMat->GetTexture(type, index, &texPath);
	if (r == aiReturn_FAILURE)
		return nullptr;

	const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texPath.C_Str());
	if (embeddedTexture != nullptr) {
		if (embeddedTexture->mHeight == 0) { // embedded file
			Texture* tex = new Texture((const unsigned char*)embeddedTexture->pcData, (size_t)embeddedTexture->mWidth, texPath.C_Str(), embeddedTexture->achFormatHint);
			return tex;
		}
		else {  // embedded raw data
			return nullptr;
		}
	}
	else {
		const char* shortName = scene->GetShortFilename(texPath.C_Str());
		std::string path = "";
		if (std::filesystem::exists(texPath.C_Str()))
			path = texPath.C_Str();
		else if (std::filesystem::exists(directory + "/" + texPath.C_Str()))
			path = directory + "/" + texPath.C_Str();
		else if (std::filesystem::exists(directory + "/" + shortName))
			path = directory + "/" + shortName;
		else if (std::filesystem::exists(directory + "/../textures/" + shortName))
			path = std::filesystem::absolute(directory + "/../textures/" + shortName).u8string();
		else
			spdlog::error("Texture {} not found", texPath.C_Str());

		if (!path.empty()) {
			Texture* tex = new Texture(path);
			if (tex->IsValid()) {
				return tex;
			}
			else {
				delete tex;
			}
		}
	}
	return nullptr;
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
	Vulkan::GetDevice()->UpdateSamplerDescriptorSet(m_materialDescSet, 1, imgInfo);
}

void Material::SetSpecularTexture(Texture* texture) {
	m_specularTex = texture;
	VkDescriptorImageInfo imgInfo = texture->GetDescriptorImageInfo();
	Vulkan::GetDevice()->UpdateSamplerDescriptorSet(m_materialDescSet, 2, imgInfo);
}

void Material::UpdateUniform() {
	MaterialUBO ubo{};
	ubo.diffuse = m_diffuseColor;
	ubo.specular = m_specularColor;
	ubo.ambient = m_ambientColor;
	ubo.shininess = m_shininess;
	ubo.emissive = m_emissiveColor;

	Vulkan::GetDevice()->UpdateUniformBuffer(m_materialMemory, 0, sizeof(MaterialUBO), &ubo);
}

void Material::Log(const std::string& prefix, fmt::memory_buffer &out) const {
	fmt::format_to(std::back_inserter(out), "{}name=\"{}\", shadingModel={}\n", prefix, GetName(), GetShadingModelName());
	LogColor("\tdiffuse  ", GetDiffuseColor(), out);
	LogColor("\tambient  ", GetAmbientColor(), out);
	LogColor("\tspecular ", GetSpecularColor(), out);
	LogColor("\temissive ", GetEmissiveColor(), out);
	fmt::format_to(std::back_inserter(out), "\tshininess = {}\n", GetShininess());
	LogTexture("\ttex.diffuse ", GetDiffuseTexture(), out);
	LogTexture("\ttex.specular", GetSpecularTexture(), out);
}

void Material::LogTexture(const std::string& prefix, const Texture* tex, fmt::memory_buffer& out) {
	if (tex && !tex->IsDefault() && tex->IsValid()) {
		if (tex->IsCreatedFromFile())
			fmt::format_to(std::back_inserter(out), "{} = \"{}\", ({}x{}x{})\n", prefix, tex->GetFilename(), tex->GetWidth(), tex->GetHeight(), tex->GetChannels());
		else
			fmt::format_to(std::back_inserter(out), "{} = \"{}\" (embedded {}), ({}x{}x{})\n", prefix, tex->GetFilename(), tex->GetFormat(), tex->GetWidth(), tex->GetHeight(), tex->GetChannels());
	}
}

void Material::LogColor(const std::string& prefix, const glm::vec3& color, fmt::memory_buffer& out) {
	fmt::format_to(std::back_inserter(out), "{} = ({:.3f}, {:.3f}, {:.3f})\n", prefix, color.r, color.g, color.b);
}
