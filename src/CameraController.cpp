#include <GLFW/glfw3.h>

#include "Window.h"
#include "Camera.h"
#include "CameraController.h"

CameraController::CameraController(Window& window, Camera& cam):
	m_window(window),
    m_cam(cam),
    m_time(0),
    m_buttonPressed(false),
    m_lastMousePosition(0.0f)
{
    m_cam.LookAt({ 6.0f, 6.0f, 6.0f });
}

void CameraController::Update(float deltaTime) {
    m_time += deltaTime;
    
    if (m_window.GetKey(GLFW_KEY_W) == GLFW_PRESS)
        m_cam.MoveForward(+1.0f * deltaTime);
    else if (m_window.GetKey(GLFW_KEY_S) == GLFW_PRESS)
        m_cam.MoveForward(-1.0f * deltaTime);
    
    if (m_window.GetKey(GLFW_KEY_D) == GLFW_PRESS)
        m_cam.MoveRight(+1.0f * deltaTime);
    else if (m_window.GetKey(GLFW_KEY_A) == GLFW_PRESS)
        m_cam.MoveRight(-1.0f * deltaTime);
    
    if (m_window.GetKey(GLFW_KEY_E) == GLFW_PRESS)
        m_cam.MoveUp(+1.0f * deltaTime);
    else if (m_window.GetKey(GLFW_KEY_Q) == GLFW_PRESS)
        m_cam.MoveUp(-1.0f * deltaTime);

    if (m_window.GetKey(GLFW_KEY_Z) == GLFW_PRESS)
        m_cam.AddPerspectiveFov(+1.0f); // Raise Fov
    else if (m_window.GetKey(GLFW_KEY_C) == GLFW_PRESS)
        m_cam.AddPerspectiveFov(-1.0f); // Lower Fov
    
    UpdateRotation();
}

void CameraController::UpdateRotation() {
    if (m_window.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        m_window.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glm::vec2 mousePos = m_window.GetMousePosition();
        glm::vec2 delta = (mousePos - m_lastMousePosition) * 0.002f;
        m_lastMousePosition = mousePos;

        if (!m_buttonPressed) {
            m_buttonPressed = true;
            return; // La primera vez que se hace click no se mueve la camara, solo se guarda a posicion donde se hizo click
        }

        float pitchDelta = delta.y * 0.4f;
        float yawDelta = delta.x * 0.4f;

        glm::quat q = glm::normalize(
            glm::cross(
                glm::angleAxis(-pitchDelta, m_cam.GetRightDirection()),
                glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))
            )
        );
        m_cam.SetForwardDirection(glm::rotate(q, m_cam.GetForwardDirection()));
    }
    else if (m_window.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        m_window.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_buttonPressed = false;
    }
}