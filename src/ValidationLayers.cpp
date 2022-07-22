#include "spdlog/spdlog.h"
#include "ValidationLayers.h"

ValidationLayers::ValidationLayers(const std::vector<const char*> layers) :
    m_layers(layers),
    m_debugCreateInfo({})
{
    if (m_enabled && !CheckSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayers::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        spdlog::debug("Validation layer: {}", pCallbackData->pMessage);
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        spdlog::info("Validation layer: {}", pCallbackData->pMessage);
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        spdlog::warn("Validation layer: {}", pCallbackData->pMessage);
    else
        spdlog::error("Validation layer: {}", pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult ValidationLayers::CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void ValidationLayers::DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void ValidationLayers::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = this; // Optional
}

void ValidationLayers::CreateDebugMessenger(VkInstance instance) {
    if (!m_enabled) return;

    m_instance = instance;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void ValidationLayers::DestroyDebugMessenger() {
    if (m_enabled) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
}

bool ValidationLayers::CheckSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_layers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void ValidationLayers::FillVkInstanceCreateInfo(VkInstanceCreateInfo& createInfo) {
    if (m_enabled) {
        PopulateDebugMessengerCreateInfo(m_debugCreateInfo);

        createInfo.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
        createInfo.ppEnabledLayerNames = m_layers.data();
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&m_debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
}

void ValidationLayers::FillVkDeviceCreateInfo(VkDeviceCreateInfo& createInfo) {
    if (m_enabled) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
        createInfo.ppEnabledLayerNames = m_layers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
}