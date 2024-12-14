#pragma once
#include <optional>

#include <vulkan/vulkan.h>

#include "Window.h"
#include "ValidationLayers.h"

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete();
};

class Device
{
public:
	Device(VkInstance instance, Window &window, ValidationLayers& validationLayers);
	~Device();

	VkDevice Get() const { return m_device; }

	VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue GetPresentQueue() const { return m_presentQueue; }

	VkSampleCountFlagBits GetMSAASamples() const { return m_msaaSamples; }
	VkCommandPool GetCommandPool() const { return m_commandPool; }
	void GetProperties(VkPhysicalDeviceProperties* props);
	void GetFormatProperties(VkFormat format, VkFormatProperties* props);
	void GetMemoryProperties(VkPhysicalDeviceMemoryProperties* props);
	QueueFamilyIndices FindQueueFamilies();
	VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }

	VkCommandPool CreateCommandPool();
	void DestroyCommandPool(VkCommandPool commandPool);
	
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void DestroyImageView(VkImageView imageView);

	VkResult CreateRenderPass(
		const VkRenderPassCreateInfo* pCreateInfo,
		VkRenderPass* pRenderPass);

	void DestroyRenderPass(VkRenderPass renderPass) { vkDestroyRenderPass(m_device, renderPass, nullptr); }

	VkResult CreatePipelineLayout(
		const VkPipelineLayoutCreateInfo* pCreateInfo,
		VkPipelineLayout* pPipelineLayout);

	VkResult CreateGraphicsPipeline(
		const VkGraphicsPipelineCreateInfo* pCreateInfo,
		VkPipeline* pPipeline);

	void DestroyPipeline(VkPipeline pipeline) { vkDestroyPipeline(m_device, pipeline, nullptr); }

	void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) { vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr); }

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void DestroyShaderModule(VkShaderModule shaderModule) { vkDestroyShaderModule(m_device, shaderModule, nullptr); }

	VkResult CreateFramebuffer(
		const VkFramebufferCreateInfo* pCreateInfo,
		VkFramebuffer* pFramebuffer);

	void DestroyFramebuffer(VkFramebuffer framebuffer) { vkDestroyFramebuffer(m_device, framebuffer, nullptr); }

	VkResult AllocateCommandBuffers(
		const VkCommandBufferAllocateInfo* pAllocateInfo,
		VkCommandBuffer* pCommandBuffers);

	VkResult CreateSemaphore(
		const VkSemaphoreCreateInfo* pCreateInfo,
		VkSemaphore* pSemaphore);

	void DestroySemaphore(VkSemaphore semaphore) { vkDestroySemaphore(m_device, semaphore, nullptr); }

	VkResult CreateFence(
		const VkFenceCreateInfo* pCreateInfo,
		VkFence* pFence);

	void DestroyFence(VkFence fence) { vkDestroyFence(m_device, fence, nullptr); }

	VkResult ResetFences(uint32_t fenceCount, const VkFence* pFences) { return vkResetFences(m_device, fenceCount, pFences); }

	VkResult WaitForFences(
		uint32_t fenceCount,
		const VkFence* pFences,
		VkBool32 waitAll,
		uint64_t timeout);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void DestroyBuffer(VkBuffer buffer) { vkDestroyBuffer(m_device, buffer, nullptr); }

	void CreateImage(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory);

	void DestroyImage(VkImage image);

	VkResult AllocateMemory(
		const VkMemoryAllocateInfo* pAllocateInfo,
		VkDeviceMemory* pMemory);

	void FreeMemory(VkDeviceMemory memory) { vkFreeMemory(m_device, memory, nullptr); }

	VkResult MapMemory(
		VkDeviceMemory memory,
		VkDeviceSize offset,
		VkDeviceSize size,
		VkMemoryMapFlags flags,
		void** ppData);

	void UnmapMemory(VkDeviceMemory memory) { vkUnmapMemory(m_device, memory); }

	VkResult CreateSampler(
		const VkSamplerCreateInfo* pCreateInfo,
		VkSampler* pSampler);

	void DestroySampler(VkSampler sampler) { vkDestroySampler(m_device, sampler, nullptr); }

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void DrawCommandBufferSubmit(
		const VkSemaphore& waitSemaphore,
		const VkPipelineStageFlags& waitDstStageMask,
		const VkCommandBuffer& commandBuffer,
		const VkSemaphore& signalSemaphore,
		VkFence fence);

	VkResult CreateDescriptorPool(
		const VkDescriptorPoolCreateInfo* pCreateInfo,
		VkDescriptorPool* pDescriptorPool);

	VkDescriptorPool CreateDescriptorPool();

	VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);

	std::vector<VkDescriptorSet> AllocateDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t count);

	void DestroyDescriptorPool(VkDescriptorPool descriptorPool) { vkDestroyDescriptorPool(m_device, descriptorPool, nullptr); }

	VkResult CreateDescriptorSetLayout(
		const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
		VkDescriptorSetLayout* pSetLayout);

	VkDescriptorSetLayout CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> bindings);

	void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr); }

	VkResult AllocateDescriptorSets(
		const VkDescriptorSetAllocateInfo* pAllocateInfo,
		VkDescriptorSet* pDescriptorSets);

	VkResult FreeDescriptorSets(
		VkDescriptorPool descriptorPool,
		uint32_t descriptorSetCount,
		const VkDescriptorSet* pDescriptorSets);

	void UpdateDescriptorSets(
		uint32_t descriptorWriteCount,
		const VkWriteDescriptorSet* pDescriptorWrites);

	void UpdateUniformDescriptorSet(VkDescriptorSet descSet, uint32_t bindingID, VkBuffer buffer, VkDeviceSize size);
	void UpdateUniformDescriptorSets(std::vector<VkDescriptorSet>& descSets, uint32_t bindingID, VkBuffer& buffer, VkDeviceSize size);
	void UpdateSamplerDescriptorSet(VkDescriptorSet descSet, uint32_t bindingID, VkDescriptorImageInfo& imageInfo);
	void UpdateUniformBuffer(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, void* data);
	size_t PadUniformBufferSize(size_t originalSize);

	VkResult WaitIdle() { return vkDeviceWaitIdle(m_device); }

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicaldevice, VkSurfaceKHR surface);

	static bool HasStencilComponent(VkFormat format);
	static void Print(int id, VkPhysicalDevice device);

private:
	const std::vector<const char*> m_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkInstance m_instance = VK_NULL_HANDLE;
	Window* m_window = nullptr;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;

	void PrintAllPhysicalDevices();
	VkPhysicalDevice SelectPhysicalDevice(VkSurfaceKHR surface);
	bool IsSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
	bool CheckExtensionSupport(VkPhysicalDevice physicalDevice);

	VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, ValidationLayers& validationLayers);
};
