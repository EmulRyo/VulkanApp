#include "VulkanApp.h"
#include "Device.h"

#include "Material.h"

Material::Material(Device& device) :
	m_device(device),
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

Material::~Material() {
	m_device.DestroyBuffer(m_materialBuffer);
	m_device.FreeMemory(m_materialMemory);
}

void Material::Init() {
	VkDescriptorPool pool = VulkanApp::GetInstance()->GetDescriptorPool();
	VkDescriptorSetLayout layout = VulkanApp::GetInstance()->GetMaterialLayout();

	m_device.CreateBuffer(
		sizeof(MaterialUBO),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_materialBuffer,
		m_materialMemory);

	m_materialDescSet = m_device.AllocateDescriptorSet(pool, layout);
	m_device.UpdateUniformDescriptorSet(m_materialDescSet, 0, m_materialBuffer, sizeof(MaterialUBO));

	SetDiffuseTexture(VulkanApp::GetInstance()->GetDummyTexture());
	SetSpecularTexture(VulkanApp::GetInstance()->GetDummyTexture());
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
	m_device.UpdateSamplerDescriptorSet(m_materialDescSet, 1, imgInfo);
}

void Material::SetSpecularTexture(Texture* texture) {
	m_specularTex = texture;
	VkDescriptorImageInfo imgInfo = texture->GetDescriptorImageInfo();
	m_device.UpdateSamplerDescriptorSet(m_materialDescSet, 2, imgInfo);
}

void Material::UpdateUniform() {
	MaterialUBO ubo{};
	ubo.diffuse = m_diffuseColor;
	ubo.specular = m_specularColor;
	ubo.ambient = m_ambientColor;
	ubo.shininess = m_shininess;
	ubo.emissive = m_emissiveColor;

	m_device.UpdateUniformBuffer(m_materialMemory, 0, sizeof(MaterialUBO), &ubo);
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
