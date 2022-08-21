#pragma once
#include <vulkan/vulkan.h>

class Device;

class RenderImage
{
public:
	RenderImage(Device& device, VkFormat format, VkExtent2D extent, VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags);
	~RenderImage();

	VkImageView GetView() const { return m_view; }

private:
	Device& m_device;
	VkImage m_image;
	VkDeviceMemory m_memory;
	VkImageView m_view;
};

