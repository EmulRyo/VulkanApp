#pragma once

#include <vulkan/vulkan.h>

class Device;
class Window;
class RenderImage;

class Swapchain
{
public:
	struct SupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

public:
	Swapchain(Device& device, Window& window);
	~Swapchain();
	
	VkSwapchainKHR Get() { return m_swapchain; };
	std::vector<VkImageView>& GetImageViews() { return m_imageViews; }
	const VkFormat& GetImageFormat() const { return m_imageFormat; }
	const VkExtent2D& GetExtent() const { return m_extent; }
	const VkFramebuffer &GetFramebuffer(size_t index) { return m_framebuffers[index]; }

	VkResult AcquireNextImage(
		uint64_t timeout,
		VkSemaphore semaphore,
		VkFence fence,
		uint32_t* pImageIndex);

	void CreateFramebuffers(const RenderImage& color, const RenderImage& depth, const VkRenderPass renderPass);

	static Swapchain::SupportDetails QuerySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

private:
	Device& m_device;
	VkSwapchainKHR m_swapchain;
	VkFormat m_imageFormat;
	VkExtent2D m_extent;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	std::vector<VkFramebuffer> m_framebuffers;

	VkSwapchainKHR Create(VkPhysicalDevice physicalDevice, Window& window);
	void CreateImageViews();
	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);
};
