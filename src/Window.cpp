#include "Window.h"

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    instance->_FramebufferResizeCB(width, height);
}

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

VkSurfaceKHR Window::GetVulkanSurface() {
    if (!m_surface)
        glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);
        
    return m_surface;
}

void Window::GetFrameBufferSize(int& width, int& height) {
    glfwGetFramebufferSize(m_window, &width, &height);
}

std::vector<const char*> Window::GetRequiredExtensions(bool enableValidationLayers) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    //first, last
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(m_window) != 0;
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::WaitEvents() {
    glfwWaitEvents();
}

void Window::EventSubscribe_OnFramebufferResize(const std::function<void(int width, int height)> func) {
    m_OnFramebufferResizeCallbackList.append(func);
}

void Window::_FramebufferResizeCB(int width, int height) {
    m_OnFramebufferResizeCallbackList(width, height);
}
