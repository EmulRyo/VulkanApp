#pragma once

#include <string>
#include <vulkan/vulkan.h>

class Device;

class Texture
{
public:
	Texture();
	Texture(const std::string &filename, bool mipmapping=true);
	~Texture();

	VkDescriptorImageInfo GetDescriptorImageInfo() const;
	const std::string& GetFilename() const { return m_filename; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetChannels() const { return m_channels; }

private:
	std::string m_filename;
	bool mipmapping;
	int m_width;
	int m_height;
	int m_channels;
	uint32_t m_mipLevels;
	VkImage m_image;
	VkDeviceMemory m_deviceMemory;
	VkImageView m_imageView;
	VkSampler m_sampler;

	void CreateImage();
	void CreateImage(const std::string& filename);
	void CreateImage(VkDeviceSize imageSize, unsigned char* pixels);
	void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void CreateImageView();
	void CreateSampler();
};

