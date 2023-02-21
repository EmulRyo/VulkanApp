#include <iostream>
#include <istream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <vector>

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Device.h"
#include "Mesh.h"
#include "Model.h"
#include "Texture.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "RenderImage.h"
#include "Shader.h"
#include "Camera.h"
#include "Axes.h"
#include "Grid.h"
#include "Prism.h"
#include "VulkanApp.h"

#define MAX_LIGHTS 8

static VulkanApp* s_instance;

struct Light {
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 attenuation; // x:constant, y:linear, z:quadratic
    glm::vec4 cutOff; // x:inner, y:outter
};

struct GlobalUBO {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
    glm::vec4 viewPos;
    Light lights[MAX_LIGHTS];
    glm::ivec4 numLights; // x:directional, y:point, z:spot
};

struct PushConstants {
    glm::mat4 model;
    glm::mat3x4 normal; // normalMatrix (3x3). Para evitar problemas de alineacion se usa una de 3x4
};

VulkanApp::VulkanApp() :
    m_window(WIDTH, HEIGHT, "Vulkan"),
    m_validationLayers({ "VK_LAYER_KHRONOS_validation" }),
    m_instance(VK_NULL_HANDLE),
    m_camController(m_window, m_cam)
{
    s_instance = this;
    spdlog::set_level(spdlog::level::level_enum::trace);

    m_window.EventSubscribe_OnFramebufferResize(std::bind(&VulkanApp::FramebufferResizeCallback, this, std::placeholders::_1, std::placeholders::_2));
    m_window.EventSubscribe_OnKey(std::bind(&VulkanApp::KeyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    initVulkan();

    m_lastTime = std::chrono::high_resolution_clock::now();
    VkExtent2D extent = m_swapchain->GetExtent();
    m_cam.SetPerspective(45.0f, extent.width / (float)extent.height, 0.01f, 100.0f);


    m_dummyTexture = new Texture(*m_device);

    GameObject* grid1 = new GameObject("Grid1");
    grid1->AddComponent<Grid>(*m_device, 10, 1.0f, 0.002f);
    m_gameObjects.push_back(grid1);

    GameObject* grid2 = new GameObject("Grid2");
    glm::vec3 color = { 0.2f, 0.2f, 0.2f };
    grid2->AddComponent<Grid>(*m_device, 20, 0.1f, 0.0018f, color);
    m_gameObjects.push_back(grid2);

    GameObject* axes = new GameObject("Axes");
    axes->AddComponent<Axes>(*m_device, 100.0f, 0.0021f);
    m_gameObjects.push_back(axes);

    GameObject* gameObject1 = NewGameObject("GameObject1", MODEL_PATH);
    m_gameObjects.push_back(gameObject1);
}

VulkanApp::~VulkanApp() {
    cleanup();
}

VulkanApp* VulkanApp::GetInstance() { 
    return s_instance; 
};

void VulkanApp::run() {
    while (!m_window.ShouldClose()) {
        ChronoTime currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastTime).count();
        m_lastTime = currentTime;

        Update(deltaTime);
        Draw(deltaTime);
    }
}

void VulkanApp::FramebufferResizeCallback(int width, int height) {
    m_framebufferResized = true;
}

void VulkanApp::KeyCallback(int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS)) {
        m_selectedPipeline = m_selectedPipeline == m_pipeline ? m_pipelineWireframe : m_pipeline;
    }
}

void VulkanApp::checkExtensions() {
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

    std::vector<const char*> requiredExtensions = m_window.GetRequiredExtensions(m_validationLayers.IsEnabled());
    spdlog::debug("Required extensions:");
    spdlog::set_pattern("%v");
    for (int i = 0; i < requiredExtensions.size(); i++) {
        spdlog::debug("\t{}", requiredExtensions[i]);
    }
    spdlog::debug("");
    spdlog::set_pattern("%+");
}

void VulkanApp::createInstance() {
    checkExtensions();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

    auto extensions = m_window.GetRequiredExtensions(m_validationLayers.IsEnabled());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    m_validationLayers.FillVkInstanceCreateInfo(createInfo);


    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanApp::initVulkan() {
    createInstance();
    m_validationLayers.CreateDebugMessenger(m_instance);

    m_window.SetVulkanInstance(m_instance);
    if (m_window.GetVulkanSurface() == VK_NULL_HANDLE)
        throw std::runtime_error("failed to create window surface!");

    m_device = new Device(m_instance, m_window, m_validationLayers);
    m_swapchain = new Swapchain(*m_device, m_window);

    createRenderPass();
    
    m_descriptorPool = m_device->CreateDescriptorPool();

    m_globalLayout = m_device->CreateDescriptorSetLayout(GetGlobalBindings());
    size_t sizeUniform = m_device->PadUniformBufferSize(sizeof(GlobalUBO));
    m_device->CreateBuffer(
        sizeUniform * MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_globalBuffer,
        m_globalMemory);
    m_globalSet = m_device->AllocateDescriptorSets(m_descriptorPool, m_globalLayout, MAX_FRAMES_IN_FLIGHT);
    m_device->UpdateUniformDescriptorSets(m_globalSet, 0, m_globalBuffer, sizeof(GlobalUBO));
    
    m_materialLayout = m_device->CreateDescriptorSetLayout(GetMaterialBindings());

    m_shader = new Shader(*m_device, MAX_FRAMES_IN_FLIGHT, "shaders/vert.spv", "shaders/frag.spv");

    createGraphicsPipeline();

    CreateRenderImages();
    m_swapchain->CreateFramebuffers(*m_color, *m_depth, m_renderPass);

    createCommandBuffers();
    createSyncObjects();
}

GameObject* VulkanApp::NewGameObject(const std::string name, std::string modelPath) {
    GameObject* gameObject = new GameObject(name);
    Model* model = gameObject->AddComponent<Model>(*m_device, modelPath);
    glm::vec3 bboxMin = model->GetBBoxMin();
    glm::vec3 bboxMax = model->GetBBoxMax();
    glm::vec3 size = { bboxMax.x - bboxMin.x, bboxMax.y - bboxMin.y, bboxMax.z - bboxMin.z };
    float maxSide = std::max(size.x, std::max(size.y, size.z));
    model->Transform.Scale = glm::vec3(1.0f / maxSide);
    model->Transform.Translation.y = (bboxMin.y / maxSide);

    return gameObject;
}

std::vector<VkDescriptorSetLayoutBinding> VulkanApp::GetGlobalBindings() {
    VkDescriptorSetLayoutBinding binding0{};
    binding0.binding = 0;
    binding0.descriptorCount = 1;
    binding0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    return { binding0 };
}

std::vector<VkDescriptorSetLayoutBinding> VulkanApp::GetMaterialBindings() {
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

void VulkanApp::recreateSwapChain() {
    int width = 0, height = 0;
    m_window.GetFrameBufferSize(width, height);
    while (width == 0 || height == 0) {
        m_window.GetFrameBufferSize(width, height);
        m_window.WaitEvents();
    }

    m_device->WaitIdle();

    cleanupSwapChain();

    m_swapchain = new Swapchain(*m_device, m_window);

    createRenderPass();
    createGraphicsPipeline();
    CreateRenderImages();
    m_swapchain->CreateFramebuffers(*m_color, *m_depth, m_renderPass);

    m_cam.SetPerspectiveAspectRatio(width / (float)height);
}

void VulkanApp::CreateRenderImages() {
    VkExtent2D extent = m_swapchain->GetExtent();
    m_color = new RenderImage(*m_device, m_swapchain->GetImageFormat(), extent, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    m_depth = new RenderImage(*m_device, FindDepthFormat(), extent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanApp::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchain->GetImageFormat();
    colorAttachment.samples = m_device->GetMSAASamples();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_swapchain->GetImageFormat();
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
    depthAttachment.samples = m_device->GetMSAASamples();
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

    if (m_device->CreateRenderPass(&renderPassInfo, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanApp::createGraphicsPipeline() {
    std::vector<VkDescriptorSetLayout> layouts = {
        m_globalLayout,
        m_materialLayout
    };
    m_pipeline = new Pipeline(*m_device, m_renderPass, m_swapchain, m_shader, { m_globalLayout, m_materialLayout });
    m_pipeline->SetMSAA(m_device->GetMSAASamples());
    m_pipeline->SetPushConstantsSize(sizeof(PushConstants));
    m_pipeline->Build();

    m_pipelineWireframe = new Pipeline(*m_pipeline);
    m_pipelineWireframe->SetWireframeMode(true);
    m_pipelineWireframe->Build();

    m_selectedPipeline = m_pipeline;
}

void VulkanApp::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (m_device->AllocateCommandBuffers(&allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanApp::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (m_device->CreateSemaphore(&semaphoreInfo, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            m_device->CreateSemaphore(&semaphoreInfo, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            m_device->CreateFence(&fenceInfo, &inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanApp::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchain->GetFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchain->GetExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_selectedPipeline->Get());

    PushConstants constants;

    for (int i = 0; i < m_gameObjects.size(); i++) {
        constants.model = m_gameObjects[i]->GetComponent<Transform>()->GetMatrix() * m_gameObjects[i]->GetComponent<Model>()->Transform.GetMatrix();
        constants.normal = glm::mat3x4(glm::transpose(glm::inverse(constants.model)));
        vkCmdPushConstants(commandBuffer, m_selectedPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &constants);
        m_gameObjects[i]->GetComponent<Model>()->Draw(commandBuffer, m_selectedPipeline->GetLayout(), m_globalSet[currentFrame]);
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

VkFormat VulkanApp::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        m_device->GetFormatProperties(format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanApp::FindDepthFormat() {
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void PrintGlmMatrix(const glm::mat4& m, const std::string& name) {
    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m, scale, orientation, translation, skew, perspective);

    glm::vec3 angles = glm::eulerAngles(orientation);
    spdlog::debug("{}", name);
    spdlog::debug("translation: {}, {}, {}", translation.x, translation.y, translation.z);
    spdlog::debug("orientation: {}, {}, {}", glm::degrees(angles.x), glm::degrees(angles.y), glm::degrees(angles.z));
    spdlog::debug("scale: {}, {}, {}", scale.x, scale.y, scale.z);
    spdlog::debug("");
}

void VulkanApp::updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    GlobalUBO global{};
    global.view = m_cam.GetView();
    global.proj = m_cam.GetProjection();
    global.viewproj = m_cam.GetProjection() * m_cam.GetView();
    global.viewPos = glm::inverse(m_cam.GetView())[3];

    global.numLights = glm::ivec4(1, 1, 1, 0); // x:directional, y:point, z:spot
    
    global.lights[0].direction = glm::vec4(glm::normalize(glm::vec3(-1, -1, -1)), 0);
    global.lights[0].ambient  = glm::vec4(0.1f);
    global.lights[0].diffuse  = glm::vec4(0.1f);
    global.lights[0].specular = glm::vec4(0.1f);

    global.lights[1].position = glm::vec4(cos(time * 0.5f), 0.2, sin(time * 0.5f), 0);
    global.lights[1].ambient  = glm::vec4(1.0f);
    global.lights[1].diffuse  = glm::vec4(1.0f);
    global.lights[1].specular = glm::vec4(1.0f);
    global.lights[1].attenuation = glm::vec4(1.0, 1.4, 3.6, 0); // x:constant, y:linear, z:quadratic
    
    global.lights[2].position = glm::vec4(0, 0.5, 0.0, 0);
    global.lights[2].direction = glm::vec4(glm::normalize(glm::vec3(0, -1, 0)), 0);
    global.lights[2].ambient = glm::vec4(1, 1, 1, 0);
    global.lights[2].diffuse = glm::vec4(1, 1, 1, 0);
    global.lights[2].specular = glm::vec4(1, 1, 1, 0);
    global.lights[2].attenuation = glm::vec4(1.0, 1.4, 3.6, 0); // x:constant, y:linear, z:quadratic
    global.lights[2].cutOff = glm::vec4(cos(12.5), cos(17.5), 0, 0); // x:inner, y:outter

    VkDeviceSize offset = currentImage * m_device->PadUniformBufferSize(sizeof(GlobalUBO));

    m_device->UpdateUniformBuffer(m_globalMemory, offset, sizeof(GlobalUBO), &global);
}

void VulkanApp::Update(float deltaTime) {
    m_window.PollEvents();
    
    m_camController.Update(deltaTime);
}

void VulkanApp::Draw(float deltaTime) {
    m_device->WaitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = m_swapchain->AcquireNextImage(UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame);

    // Only reset the fence if we are submitting work
    m_device->ResetFences(1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    m_device->DrawCommandBufferSubmit(
        imageAvailableSemaphores[currentFrame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        commandBuffers[currentFrame],
        renderFinishedSemaphores[currentFrame],
        inFlightFences[currentFrame]);

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    VkSwapchainKHR swapChains[] = { m_swapchain->Get() };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(m_device->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
        m_framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApp::cleanupSwapChain() {
    delete m_color;
    delete m_depth;
    delete m_swapchain;

    delete m_pipeline;
    delete m_pipelineWireframe;
    m_device->DestroyRenderPass(m_renderPass);
}

void VulkanApp::cleanup() {
    m_device->WaitIdle();

    cleanupSwapChain();

    for (int i=0; i<m_gameObjects.size(); i++)
        m_gameObjects[i]->Dispose();

    delete m_dummyTexture;

    m_device->DestroyBuffer(m_globalBuffer);
    m_device->FreeMemory(m_globalMemory);
    m_device->DestroyDescriptorSetLayout(m_globalLayout);
    m_device->DestroyDescriptorSetLayout(m_materialLayout);
    m_device->DestroyDescriptorPool(m_descriptorPool);

    delete m_shader;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_device->DestroySemaphore(renderFinishedSemaphores[i]);
        m_device->DestroySemaphore(imageAvailableSemaphores[i]);
        m_device->DestroyFence(inFlightFences[i]);
    }
        
    delete m_device;
    m_validationLayers.DestroyDebugMessenger();

    vkDestroySurfaceKHR(m_instance, m_window.GetVulkanSurface(), nullptr);
    vkDestroyInstance(m_instance, nullptr);
}
