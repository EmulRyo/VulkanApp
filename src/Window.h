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

	GLFWwindow* GetGLFWHandle() const { return m_window; };
	void SetVulkanInstance(VkInstance instance) { m_instance = instance; };
	VkSurfaceKHR GetVulkanSurface();
	void GetFrameBufferSize(int& width, int& height) const;
	float GetAspectRatio() const;
	std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers) const;
	bool ShouldClose() const;
	void PollEvents() const;
	void WaitEvents() const;
	void EventSubscribe_OnFramebufferResize(const std::function<void(int width, int height)> func);
	void EventSubscribe_OnKey(const std::function<void(int key, int scancode, int action, int mods)> func);
	int  GetKey(int key) const { return glfwGetKey(m_window, key); }
	int  GetMouseButton(int button) const { return glfwGetMouseButton(m_window, button); }
	glm::vec2 GetMousePosition() const {
		double x, y;
		glfwGetCursorPos(m_window, &x, &y);
		return { (float)x, (float)y };
	}
	void SetInputMode(int mode, int value) { glfwSetInputMode(m_window, mode, value); }

	void _FramebufferResizeCB(int width, int height);
	void _KeyCB(int key, int scancode, int action, int mods);

private:
	GLFWwindow* m_window = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	int m_width, m_height;

	eventpp::CallbackList<void(int width, int height)> m_OnFramebufferResizeCallbackList;
	eventpp::CallbackList<void(int key, int scancode, int action, int mods)> m_OnKeyCallbackList;
};

