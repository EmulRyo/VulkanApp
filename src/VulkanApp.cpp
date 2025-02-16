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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "imgui.h"

#include "nfd.h"

#include "Device.h"
#include "Mesh.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"
#include "Axes.h"
#include "Grid.h"
#include "Prism.h"
#include "Vulkan.h"
#include "VulkanApp.h"

VulkanApp::VulkanApp() :
    m_window(WIDTH, HEIGHT, "Vulkan"),
    m_camController(m_window, m_cam),
    m_fps(0.5f),
    m_deltaTime(1.0f/60.0f),
    m_showGrid(true),
    m_showAxis(true),
    m_vSync(true),
    m_selectedShader(0)
{
    spdlog::set_level(spdlog::level::level_enum::trace);

    Vulkan::Init(m_window, m_vSync);

    Vulkan::ImGuiInit();

    NFD_Init();

    m_window.EventSubscribe_OnFramebufferResize(std::bind(&VulkanApp::FramebufferResizeCallback, this, std::placeholders::_1, std::placeholders::_2));
    m_window.EventSubscribe_OnKey(std::bind(&VulkanApp::KeyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    m_cam.SetPerspective(45.0f, m_window.GetAspectRatio(), 0.01f, 100.0f);

    m_grid1 = new GameObject("Grid1");
    m_grid1->AddComponent<Grid>(10, 1.0f, 0.002f);

    m_grid2 = new GameObject("Grid2");
    glm::vec3 color = { 0.2f, 0.2f, 0.2f };
    m_grid2->AddComponent<Grid>(20, 0.1f, 0.0018f, color);

    m_axes = new GameObject("Axes");
    m_axes->AddComponent<Axes>(100.0f, 0.0021f);
}

VulkanApp::~VulkanApp() {
    Cleanup();
}

void VulkanApp::run() {
    while (!m_window.ShouldClose()) {
        m_timerFrame.Start();

        Update(m_deltaTime);
        Draw(m_deltaTime);

        if (m_cleanModels)
            CleanModels();

        if (m_changeModel)
            ChangeModel();

        m_deltaTime = m_timerFrame.Stop();
    }
}

void VulkanApp::FramebufferResizeCallback(int width, int height) {
    if (height > 0)
        m_cam.SetPerspectiveAspectRatio(width / (float)height);
}

void VulkanApp::KeyCallback(int key, int scancode, int action, int mods) {

}

GameObject* VulkanApp::NewGameObject(const std::string name, std::string modelPath) {
    GameObject* gameObject = new GameObject(name);
    Model* model = gameObject->AddComponent<Model>(modelPath);
    glm::vec3 bboxMin = model->GetBBoxMin();
    glm::vec3 bboxMax = model->GetBBoxMax();
    glm::vec3 size = { bboxMax.x - bboxMin.x, bboxMax.y - bboxMin.y, bboxMax.z - bboxMin.z };
    float maxSide = std::max(size.x, std::max(size.y, size.z));
    model->Transform.Scale = glm::vec3(1.0f / maxSide);
    model->Transform.Translation.y = (bboxMin.y / maxSide);

    return gameObject;
}

void VulkanApp::DrawGameObject(GameObject *gameObject) {
    glm::mat4 matrix = gameObject->GetComponent<Transform>()->GetMatrix();
    gameObject->GetComponent<Model>()->Draw(matrix);
}

static void PrintGlmMatrix(const glm::mat4& m, const std::string& name) {
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

void VulkanApp::UpdateUniformBuffer() {
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

    Vulkan::UpdateUniformBuffer(sizeof(GlobalUBO), &global);
}

void VulkanApp::Update(float deltaTime) {
    m_window.PollEvents();
    
    m_camController.Update(deltaTime);
    m_fps.Update(deltaTime);
}

void VulkanApp::Draw(float deltaTime) {
    Vulkan::BeginDrawing();

    UpdateUniformBuffer();

    if (m_showGrid) {
        DrawGameObject(m_grid1);
        DrawGameObject(m_grid2);
    }
    if (m_showAxis)
        DrawGameObject(m_axes);

    for (int i = 0; i < m_gameObjects.size(); i++) {
        DrawGameObject(m_gameObjects[i]);
    }

    GuiDraw();

    Vulkan::EndDrawing();
}

void VulkanApp::Cleanup() {
    Vulkan::WaitIdle();

    m_grid1->Dispose();
    m_grid2->Dispose();
    m_axes->Dispose();
    for (int i=0; i<m_gameObjects.size(); i++)
        m_gameObjects[i]->Dispose();

    Vulkan::ImGuiCleanup();

    Vulkan::Cleanup();

    NFD_Quit();
}

void VulkanApp::GuiDraw() {
    Vulkan::ImGuiBeginDrawing();

    ImGui::Begin("Vulkan App");

    if (ImGui::Button("Open file")) {
        OpenFileDialog();
    }

    ImGui::Checkbox("Show grid", &m_showGrid);
    ImGui::Checkbox("Show axis", &m_showAxis);
    if (ImGui::Checkbox("VSync", &m_vSync)) {
        Vulkan::SetVSync(m_vSync);
    }
    if (ImGui::Combo("Shader", &m_selectedShader, "Phong\0Unlit\0")) {
        Vulkan::SetPipeline(m_selectedShader);
    }
    ImGui::Text("FPS: %d (%.2f ms)", m_fps.GetFPS(), m_fps.GetFrametime()*1000.0f);

    //bool open = true;
    //ImGui::ShowDemoWindow(&open);

    ImGui::End();

    Vulkan::ImGuiEndDrawing();
}

void VulkanApp::OpenFileDialog() {
    nfdu8char_t* outPath;
    nfdu8filteritem_t filters[1] = { { "3D Models", "fbx,obj,gltf,glb" } };
    nfdopendialogu8args_t args = { 0 };
    args.filterList = filters;
    args.filterCount = 1;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        m_changeModel = true;
        m_modelPath = outPath;
        NFD_FreePathU8(outPath);
    }
    else if (result == NFD_ERROR)
        printf("Error: %s\n", NFD_GetError());
}

void VulkanApp::CleanModels() {
    Vulkan::WaitIdle();
    for (int i = 0; i < m_gameObjects.size(); i++) {
        m_gameObjects[i]->Dispose();
        delete m_gameObjects[i];
    }
    m_gameObjects.clear();

    m_cleanModels = false;
}

void VulkanApp::ChangeModel() {
    CleanModels();
    GameObject* gameObject1 = NewGameObject("GameObject1", m_modelPath);
    m_gameObjects.push_back(gameObject1);

    m_changeModel = false;
}

