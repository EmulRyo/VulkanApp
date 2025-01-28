#include <stdexcept>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "Device.h"
#include "Pipeline.h"
#include "RenderImage.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Texture.h"
#include "ValidationLayers.h"
#include "Window.h"

#include "Vulkan.h"

void CreateInstance(Window& window);
void CreateRenderPass();
VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat FindDepthFormat();
std::vector<VkDescriptorSetLayoutBinding> GetGlobalBindings();
std::vector<VkDescriptorSetLayoutBinding> GetMaterialBindings();
void CreateGraphicsPipeline();
void CreateRenderImages();
void CheckExtensions(const Window& window);
void CreateCommandBuffers();
void CreateSyncObjects();
void CleanupSwapChain();
void FramebufferResizeCallback(int width, int height);

VkInstance g_instance;
ValidationLayers g_validationLayers({ "VK_LAYER_KHRONOS_validation" });
Device* g_device;
Swapchain* g_swapchain;
VkDescriptorPool g_descriptorPool;
VkDescriptorSetLayout g_globalLayout, g_materialLayout;
VkBuffer g_globalBuffer;
VkDeviceMemory g_globalMemory;
std::vector<VkDescriptorSet> g_globalSet;
RenderImage* g_color;
RenderImage* g_depth;
VkRenderPass g_renderPass;
Pipeline* g_phongPipeline;
Pipeline* g_unlitPipeline;
Pipeline* g_selectedPipeline;
Shader* g_phongShader;
Shader* g_unlitShader;
Texture* g_dummyTexture;

std::vector<VkCommandBuffer> commandBuffers;
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;

int g_guiMinImageCount = 2;
VkDescriptorPool g_guiDescriptorPool = VK_NULL_HANDLE;

uint32_t g_imageIndex;
uint32_t currentFrame = 0;

Window* g_window = nullptr;
bool g_framebufferResized = false;
bool g_vSyncChanged = false;
bool g_vSync = true;

void Vulkan::Init(Window &window, bool vSync) {
    g_window = &window;
    g_vSync = vSync;
    CreateInstance(window);
    g_validationLayers.CreateDebugMessenger(g_instance);

    window.SetVulkanInstance(g_instance);
    if (window.GetVulkanSurface() == VK_NULL_HANDLE)
        throw std::runtime_error("failed to create window surface!");

    g_device = new Device(g_instance, window, g_validationLayers);
    g_swapchain = new Swapchain(*g_device, window, vSync);

    CreateRenderPass();

    g_descriptorPool = g_device->CreateDescriptorPool();

    g_globalLayout = g_device->CreateDescriptorSetLayout(GetGlobalBindings());
    size_t sizeUniform = g_device->PadUniformBufferSize(sizeof(GlobalUBO));
    g_device->CreateBuffer(
        sizeUniform * MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        g_globalBuffer,
        g_globalMemory);
    g_globalSet = g_device->AllocateDescriptorSets(g_descriptorPool, g_globalLayout, MAX_FRAMES_IN_FLIGHT);
    g_device->UpdateUniformDescriptorSets(g_globalSet, 0, g_globalBuffer, sizeof(GlobalUBO));

    g_materialLayout = g_device->CreateDescriptorSetLayout(GetMaterialBindings());

    g_phongShader = new Shader(*g_device, "shaders/phong.vert.spv", "shaders/phong.frag.spv");
    g_unlitShader = new Shader(*g_device, "shaders/unlit.vert.spv", "shaders/unlit.frag.spv");

    CreateGraphicsPipeline();

    CreateRenderImages();
    g_swapchain->CreateFramebuffers(*g_color, *g_depth, g_renderPass);

    CreateCommandBuffers();
    CreateSyncObjects();

    g_dummyTexture = new Texture();

    g_window->EventSubscribe_OnFramebufferResize(&FramebufferResizeCallback);
}

void CreateInstance(Window& window) {
    CheckExtensions(window);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

    auto extensions = window.GetRequiredExtensions(g_validationLayers.IsEnabled());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    g_validationLayers.FillVkInstanceCreateInfo(createInfo);

    if (vkCreateInstance(&createInfo, nullptr, &g_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = g_swapchain->GetImageFormat();
    colorAttachment.samples = g_device->GetMSAASamples();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = g_swapchain->GetImageFormat();
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = g_device->GetMSAASamples();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (g_device->CreateRenderPass(&renderPassInfo, &g_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        g_device->GetFormatProperties(format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat FindDepthFormat() {
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

std::vector<VkDescriptorSetLayoutBinding> GetGlobalBindings() {
    VkDescriptorSetLayoutBinding binding0{};
    binding0.binding = 0;
    binding0.descriptorCount = 1;
    binding0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    return { binding0 };
}

std::vector<VkDescriptorSetLayoutBinding> GetMaterialBindings() {
    VkDescriptorSetLayoutBinding binding0{};
    binding0.binding = 0;
    binding0.descriptorCount = 1;
    binding0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutBinding binding1{};
    binding1.binding = 1;
    binding1.descriptorCount = 1;
    binding1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutBinding binding2{};
    binding2.binding = 2;
    binding2.descriptorCount = 1;
    binding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    return { binding0, binding1, binding2 };
}

void CreateGraphicsPipeline() {
    std::vector<VkDescriptorSetLayout> layouts = {
        g_globalLayout,
        g_materialLayout
    };
    g_phongPipeline = new Pipeline(*g_device, g_renderPass, g_swapchain, g_phongShader, { g_globalLayout, g_materialLayout });
    g_phongPipeline->SetMSAA(g_device->GetMSAASamples());
    g_phongPipeline->SetPushConstantsSize(sizeof(PushConstants));
    g_phongPipeline->Build();

    g_unlitPipeline = new Pipeline(*g_phongPipeline);
    g_unlitPipeline->SetShader(g_unlitShader);
    g_unlitPipeline->Build();

    g_selectedPipeline = g_phongPipeline;
}

void CreateRenderImages() {
    VkExtent2D extent = g_swapchain->GetExtent();
    g_color = new RenderImage(*g_device, g_swapchain->GetImageFormat(), extent, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    g_depth = new RenderImage(*g_device, FindDepthFormat(), extent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void CheckExtensions(const Window &window) {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    spdlog::debug("Available extensions:");
    spdlog::set_pattern("%v");
    for (const auto& extension : extensions) {
        spdlog::debug("\t{} [v{}]", extension.extensionName, extension.specVersion);
    }
    spdlog::debug("");
    spdlog::set_pattern("%+");

    std::vector<const char*> requiredExtensions = window.GetRequiredExtensions(g_validationLayers.IsEnabled());
    spdlog::debug("Required extensions:");
    spdlog::set_pattern("%v");
    for (int i = 0; i < requiredExtensions.size(); i++) {
        spdlog::debug("\t{}", requiredExtensions[i]);
    }
    spdlog::debug("");
    spdlog::set_pattern("%+");
}

void CreateCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = g_device->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (g_device->AllocateCommandBuffers(&allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CreateSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (g_device->CreateSemaphore(&semaphoreInfo, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            g_device->CreateSemaphore(&semaphoreInfo, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            g_device->CreateFence(&fenceInfo, &inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Vulkan::SetVSync(bool value) {
    g_vSyncChanged = true;
    g_vSync = value;
}

void Vulkan::SetPipeline(int id) {
    g_selectedPipeline = id == 0 ? g_phongPipeline : g_unlitPipeline;
}

void RecreateSwapChain() {
    int width = 0, height = 0;
    g_window->GetFrameBufferSize(width, height);
    while (width == 0 || height == 0) {
        g_window->GetFrameBufferSize(width, height);
        g_window->WaitEvents();
    }

    g_device->WaitIdle();

    CleanupSwapChain();

    g_swapchain = new Swapchain(*g_device, *g_window, g_vSync);

    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateRenderImages();
    g_swapchain->CreateFramebuffers(*g_color, *g_depth, g_renderPass);

    g_framebufferResized = false;
    g_vSyncChanged = false;
}

void FramebufferResizeCallback(int width, int height) {
    g_framebufferResized = true;
}

Texture* Vulkan::GetDummyTexture() {
    return g_dummyTexture;
}

Device* Vulkan::GetDevice() {
    return g_device;
}

void Vulkan::BeginDrawing() {
    g_device->WaitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = g_swapchain->AcquireNextImage(UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &g_imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work
    g_device->ResetFences(1, &inFlightFences[currentFrame]);

    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.01f, 0.01f, 0.01f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = g_renderPass;
    renderPassInfo.framebuffer = g_swapchain->GetFramebuffer(g_imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = g_swapchain->GetExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_selectedPipeline->Get());
}

void Vulkan::Draw(glm::mat4 matrix, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount, VkDescriptorSet materialDescSet) {
    
    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

    PushConstants constants{};
    constants.model = matrix;
    constants.normal = glm::mat3x4(glm::transpose(glm::inverse(matrix)));
    vkCmdPushConstants(commandBuffer, g_selectedPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &constants);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    std::vector<VkDescriptorSet> combinedDescSets;
    combinedDescSets.push_back(g_globalSet[currentFrame]);
    if (materialDescSet != nullptr)
        combinedDescSets.push_back(materialDescSet);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_selectedPipeline->GetLayout(), 0, (uint32_t)combinedDescSets.size(), combinedDescSets.data(), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Vulkan::EndDrawing() {
    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    g_device->DrawCommandBufferSubmit(
        imageAvailableSemaphores[currentFrame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        commandBuffers[currentFrame],
        renderFinishedSemaphores[currentFrame],
        inFlightFences[currentFrame]);

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    VkSwapchainKHR swapChains[] = { g_swapchain->Get() };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &g_imageIndex;
    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(g_device->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_framebufferResized || g_vSyncChanged) {
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::UpdateUniformBuffer(size_t bufferSize, void *data) {
    VkDeviceSize offset = currentFrame * g_device->PadUniformBufferSize(bufferSize);
    g_device->UpdateUniformBuffer(g_globalMemory, offset, bufferSize, data);
}

VkDescriptorPool Vulkan::GetDescriptorPool() { return g_descriptorPool; }
VkDescriptorSetLayout Vulkan::GetMaterialLayout() { return g_materialLayout; }

void Vulkan::WaitIdle() {
    g_device->WaitIdle();
}

void CleanupSwapChain() {
    delete g_color;
    delete g_depth;
    delete g_swapchain;

    delete g_phongPipeline;
    delete g_unlitPipeline;
    g_device->DestroyRenderPass(g_renderPass);
}

void Vulkan::Cleanup() {
    g_device->WaitIdle();

    CleanupSwapChain();

    delete g_dummyTexture;

    g_device->DestroyBuffer(g_globalBuffer);
    g_device->FreeMemory(g_globalMemory);
    g_device->DestroyDescriptorSetLayout(g_globalLayout);
    g_device->DestroyDescriptorSetLayout(g_materialLayout);
    g_device->DestroyDescriptorPool(g_descriptorPool);

    delete g_phongShader;
    delete g_unlitShader;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        g_device->DestroySemaphore(renderFinishedSemaphores[i]);
        g_device->DestroySemaphore(imageAvailableSemaphores[i]);
        g_device->DestroyFence(inFlightFences[i]);
    }

    delete g_device;
    g_validationLayers.DestroyDebugMessenger();

    vkDestroySurfaceKHR(g_instance, g_window->GetVulkanSurface(), nullptr);
    vkDestroyInstance(g_instance, nullptr);
}

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void Vulkan::ImGuiInit() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Create Descriptor Pool
    // The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
    // If you wish to load e.g. additional textures you may need to alter pools sizes.
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        g_device->CreateDescriptorPool(&pool_info, &g_guiDescriptorPool);
    }

    QueueFamilyIndices indices = g_device->FindQueueFamilies();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(g_window->GetGLFWHandle(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_instance;
    init_info.PhysicalDevice = g_device->GetPhysicalDevice();
    init_info.Device = g_device->Get();
    init_info.QueueFamily = indices.graphicsFamily.value();
    init_info.Queue = g_device->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = g_guiDescriptorPool;
    init_info.RenderPass = g_renderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_guiMinImageCount;
    init_info.ImageCount = 2;
    init_info.MSAASamples = g_device->GetMSAASamples();
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);
}

void Vulkan::ImGuiBeginDrawing() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Vulkan::ImGuiEndDrawing() {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[currentFrame]);
}

void Vulkan::ImGuiCleanup() {
    g_device->WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    g_device->DestroyDescriptorPool(g_guiDescriptorPool);
}
