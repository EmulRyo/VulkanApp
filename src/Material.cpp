#include "VulkanApp.h"
#include "Device.h"

#include "Material.h"

Material::Material(Device& device) :
	m_device(device),
	m_name("No name"),
	m_diffuseColor(glm::vec3(1)),
	m_specularColor(glm::vec3(0)),
	m_ambientColor(glm::vec3(0.1)),
	m_emissiveColor(glm::vec3(0)),
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
}

void Material::SetDiffuseTexture(Texture* texture) {
	m_diffuseTex = texture;
	VkDescriptorImageInfo imgInfo = texture->GetDescriptorImageInfo();
	m_device.UpdateSamplerDescriptorSet(m_materialDescSet, 1, imgInfo);
}

void Material::UpdateUniform() {
	MaterialUBO ubo{};
	ubo.diffuse = m_diffuseColor;
	ubo.specular = m_specularColor;
	ubo.ambient = m_ambientColor;
	ubo.shininess = m_shininess;

	m_device.UpdateUniformBuffer(m_materialMemory, 0, sizeof(MaterialUBO), &ubo);
}
