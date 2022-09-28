#pragma once

#include "GameObject.h"

class Camera  {
public:
	Camera() = default;
	Camera(const Camera&) = default;
	~Camera() = default;

	void SetPerspective(float fovV, float aspectRatio, float zNear, float zFar) { 
		m_fov = fovV; m_aspectRatio = aspectRatio; m_near = zNear; m_far = zFar;
		UpdateProjection();
	}

	void SetPerspectiveAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; UpdateProjection(); }

	void AddPerspectiveFov(float amount) { m_fov = glm::clamp(m_fov + amount, 2.0f, 178.0f); UpdateProjection(); }

	void LookAt(const glm::vec3& position, const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f)) {
		m_position = position;
		m_forward = glm::normalize(target - position);
		m_view = glm::lookAt(position, target, m_worldUp);
	}

	void LookDir(const glm::vec3& position, const glm::vec3& dir) {
		m_position = position;
		m_forward = glm::normalize(dir);
		m_view = glm::lookAt(position, position + dir, m_worldUp);
	}

	void MoveForward(float amount) {
		m_position = m_position + m_forward * amount;
		m_view = glm::lookAt(m_position, m_position + m_forward, m_worldUp);
	}

	void MoveRight(float amount) {
		glm::vec3 right = GetRightDirection();
		m_position = m_position + right * amount;
		m_view = glm::lookAt(m_position, m_position + m_forward, m_worldUp);
	}

	void MoveUp(float amount) {
		glm::vec3 up = GetUpDirection();
		m_position = m_position + up * amount;
		m_view = glm::lookAt(m_position, m_position + m_forward, m_worldUp);
	}

	void SetForwardDirection(const glm::vec3 forward) {
		m_forward = forward;
		m_view = glm::lookAt(m_position, m_position + m_forward, m_worldUp);
	}

	glm::vec3 GetForwardDirection() { return m_forward; }
	glm::vec3 GetRightDirection() { return glm::normalize(glm::cross(m_forward, m_worldUp)); }
	glm::vec3 GetUpDirection() { return glm::normalize(glm::cross(GetRightDirection(), m_forward)); }

	const glm::mat4& GetProjection() const { return m_proj; }
	const glm::mat4& GetView() const { return m_view; }

private:
	float m_fov = 45.0f;
	float m_near = 0.01f, m_far = 100.0f;
	float m_aspectRatio = 0.0f;
	glm::vec3 m_worldUp = { 0.0f, 1.0f, 0.0f };
	glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_forward = {0.0f, 0.0f, -1.0f};
	glm::mat4 m_proj = glm::mat4(1.0f);
	glm::mat4 m_view = glm::mat4(1.0f);

	void UpdateProjection() {
		m_proj = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
		// GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted.
		// The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in the projection matrix.
		// If you don't do this, then the image will be rendered upside down.
		m_proj[1][1] *= -1.0f;
	}
};