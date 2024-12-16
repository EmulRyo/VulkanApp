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
	Swapchain(Device& device, Window& window, bool vSync);
	~Swapchain();
	
	VkSwapchainKHR Get() const { return m_swapchain; };
	std::vector<VkImageView>& GetImageViews() { return m_imageViews; }
	const VkFormat& GetImageFormat() const { return m_imageFormat; }
	const VkExtent2D& GetExtent() const { return m_extent; }
	const VkFramebuffer &GetFramebuffer(size_t index) { return m_framebuffers[index]; }
	bool IsVSyncEnabled() const;

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
	bool m_vSync;

	VkSwapchainKHR Create(VkPhysicalDevice physicalDevice, Window& window, bool vSync);
	void CreateImageViews();
	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vSync);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);
};
