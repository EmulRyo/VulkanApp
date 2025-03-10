#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include "Window.h"
#include "Camera.h"
#include "CameraController.h"
#include "Timer.h"
#include "FPS.h"

class Device;
class Model;
class Axes;
class Prism;

class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();

    void run();

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    Window m_window;

    int m_selectedShader;

    std::vector<GameObject *> m_gameObjects;

    Camera m_cam;
    CameraController m_camController;
    GameObject *m_grid1, *m_grid2;
    GameObject *m_axes;
    float m_deltaTime;
    Timer m_timerFrame;
    FPS m_fps;
    bool m_vSync;
    bool m_showGrid;
    bool m_showAxis;

    bool m_changeModel = false;
    bool m_cleanModels = false;
    std::string m_modelPath;

    void OpenFileDialog();
    void ChangeModel();
    GameObject* NewGameObject(const std::string name, std::string modelPath);
    void UpdateUniformBuffer();

    void Update(float deltaTime);
    void Draw(float deltaTime);
    void DrawGameObject(GameObject* gameObject);
    void GuiDraw();

    void CleanModels();
    void Cleanup();

    void FramebufferResizeCallback(int width, int height);
    void KeyCallback(int key, int scancode, int action, int mods);
};
