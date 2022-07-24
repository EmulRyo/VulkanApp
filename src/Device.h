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

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Device
{
public:
	Device(VkInstance instance, Window &window, ValidationLayers& validationLayers);
	~Device();

	VkDevice GetHandle() { return m_device; };
	VkResult WaitIdle();
	VkQueue GetGraphicsQueue() { return m_graphicsQueue; }
	VkQueue GetPresentQueue() { return m_presentQueue; }

	VkFormat GetSwapChainImageFormat() { return m_swapChainImageFormat; }
	VkExtent2D GetSwapChainExtent() { return m_swapChainExtent; }
	VkSampleCountFlagBits GetMSAASamples() { return m_msaaSamples; };
	VkSwapchainKHR GetSwapChain() { return m_swapChain; };
	std::vector<VkImageView>& GetSwapChainImageViews() { return m_swapChainImageViews; }
	void GetProperties(VkPhysicalDeviceProperties* props);
	void GetFormatProperties(VkFormat format, VkFormatProperties* props);
	void GetMemoryProperties(VkPhysicalDeviceMemoryProperties* props);
	QueueFamilyIndices FindQueueFamilies();
	void CreateSwapChain();
	void CreateImageViews();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

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

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;

	void PickPhysicalDevice(VkSurfaceKHR surface);
	bool IsSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicaldevice, VkSurfaceKHR surface);
	bool CheckExtensionSupport(VkPhysicalDevice physicalDevice);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	void CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, ValidationLayers& validationLayers);
	void CreateSwapChain(VkPhysicalDevice physicalDevice, Window& window);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);
};

