#pragma once

#include "Components.h"

class Model;

class GameObject {
public:
	GameObject() = default;
	GameObject(const GameObject&) = default;
	virtual ~GameObject() = default;

	TransformComponent Transform;
	Model* Model=nullptr;	
};