#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <spdlog/spdlog.h>

struct TransformComponent
{
	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3& translation)
		: Translation(translation) {}

	glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

	glm::mat4 GetMatrix() const {
		glm::vec3 translation = Translation;
		translation.y *= -1.0f;
		return glm::toMat4(glm::quat(Rotation))
			* glm::translate(glm::mat4(1.0f), translation)
			* glm::scale(glm::mat4(1.0f), Scale);
	}

	glm::vec3 Forward() { return glm::toMat3(glm::quat(Rotation)) * glm::vec3(0.0f, 1.0f, 0.0f); }
	glm::vec3 Up()		{ return glm::toMat3(glm::quat(Rotation)) * glm::vec3(0.0f, 0.0f, 1.0f); }
	glm::vec3 Right()	{ return glm::toMat3(glm::quat(Rotation)) * glm::vec3(1.0f, 0.0f, 0.0f); }

	void Print(const std::string& name) {
		spdlog::debug("{}", name);
		spdlog::debug("translation: {}, {}, {}", Translation.x, Translation.y, Translation.z);
		spdlog::debug("rotation: {}, {}, {}", glm::degrees(Rotation.x), glm::degrees(Rotation.y), glm::degrees(Rotation.z));
		spdlog::debug("scale: {}, {}, {}", Scale.x, Scale.y, Scale.z);
		spdlog::debug("");
	}
};