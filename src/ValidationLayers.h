#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class ValidationLayers
{
public:
    ValidationLayers(const std::vector<const char*> layers);
    ~ValidationLayers() { }

    bool IsEnabled() { return m_enabled; }
    bool CheckSupport();
    void FillVkInstanceCreateInfo(VkInstanceCreateInfo &createInfo);
    void FillVkDeviceCreateInfo(VkDeviceCreateInfo& createInfo);
    void CreateDebugMessenger(VkInstance instance);
    void DestroyDebugMessenger();

private:
    const std::vector<const char*> m_layers;
#ifdef NDEBUG
    const bool m_enabled = false;
#else
    const bool m_enabled = true;
#endif
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo;

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    static VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);
};

