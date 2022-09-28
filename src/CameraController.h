#pragma once

#include <glm/glm.hpp>

class Window;
class Camera;

class CameraController {
public:
	CameraController(Window& window, Camera& cam);
	void Update(float deltaTime);

private:
	Window& m_window;
	Camera& m_cam;
	float m_time;
	glm::vec2 m_lastMousePosition;
	bool m_buttonPressed;

	void UpdateRotation();
};