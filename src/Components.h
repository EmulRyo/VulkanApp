#pragma once

enum class ComponentType {
	Transform,
	Model
};

struct Component {
	virtual ~Component() {};
	virtual ComponentType GetType() = 0;
};