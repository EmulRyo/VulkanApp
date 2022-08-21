#include "Device.h"
#include "RenderImage.h"

RenderImage::RenderImage(Device& device, VkFormat format, VkExtent2D extent, VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags):
	m_device(device), 
	m_image(VK_NULL_HANDLE), 
	m_memory(VK_NULL_HANDLE), 
	m_view(VK_NULL_HANDLE)
{
	device.CreateImage(extent.width, extent.height, 1, device.GetMSAASamples(), format, VK_IMAGE_TILING_OPTIMAL, usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);
	m_view = device.CreateImageView(m_image, format, aspectFlags, 1);
}

RenderImage::~RenderImage() {
	m_device.DestroyImageView(m_view);
	m_device.DestroyImage(m_image);
	m_device.FreeMemory(m_memory);
}