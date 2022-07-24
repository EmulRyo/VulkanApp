#pragma once

#pragma warning( push )
#pragma warning( disable: 26812 )
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma warning( pop )

#include <eventpp/callbacklist.h>

class Window
{
public:
	Window(int width, int height, const char* title);
	~Window();

	void SetVulkanInstance(VkInstance instance) { m_instance = instance; };
	VkSurfaceKHR GetVulkanSurface();
	void GetFrameBufferSize(int& width, int& height);
	std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers);
	bool ShouldClose();
	void PollEvents();
	void WaitEvents();
	void EventSubscribe_OnFramebufferResize(const std::function<void(int width, int height)> func);

	void _FramebufferResizeCB(int width, int height);

private:
	GLFWwindow* m_window = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	int m_width, m_height;

	eventpp::CallbackList<void(int width, int height)> m_OnFramebufferResizeCallbackList;
};

