#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <eventpp/callbacklist.h>

class Window
{
public:
	Window(int width, int height, const char* title);
	~Window();

	VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface);
	void GetFrameBufferSize(int& width, int& height);
	bool ShouldClose();
	void PollEvents();
	void WaitEvents();
	void EventSubscribe_OnFramebufferResize(const std::function<void(int width, int height)> func);

	void _FramebufferResizeCB(int width, int height);

private:
	GLFWwindow* m_window = nullptr;
	int m_width, m_height;

	eventpp::CallbackList<void(int width, int height)> m_OnFramebufferResizeCallbackList;
};

