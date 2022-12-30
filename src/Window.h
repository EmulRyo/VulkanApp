#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

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
	int  GetKey(int key) { return glfwGetKey(m_window, key); }
	int  GetMouseButton(int button) { return glfwGetMouseButton(m_window, button); }
	glm::vec2 GetMousePosition() {
		double x, y;
		glfwGetCursorPos(m_window, &x, &y);
		return { (float)x, (float)y };
	}
	void SetInputMode(int mode, int value) { glfwSetInputMode(m_window, mode, value); }

	void _FramebufferResizeCB(int width, int height);

private:
	GLFWwindow* m_window = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	int m_width, m_height;

	eventpp::CallbackList<void(int width, int height)> m_OnFramebufferResizeCallbackList;
};

