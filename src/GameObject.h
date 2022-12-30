#pragma once

#include "Components.h"

class Model;

class GameObject {
public:
	GameObject(const std::string &name) {
		m_name = name;
		AddComponent(new TransformComponent);
	};
	GameObject(const GameObject&) = default;
	virtual ~GameObject() {
		Dispose();
	};

	void Dispose() {
		if (m_components.size() > 0) {
			printf("Delete %s\n", m_name.c_str());
			for (auto it = m_components.begin(); it != m_components.end(); ++it) {
				delete it->second;
			}
			m_components.clear();
		}
	}

	void AddComponent(Component* component) { m_components.emplace(component->GetType(), component); }
	
	template <class T>
	T* GetComponent() {
		auto it = m_components.find(T::GetTypeStatic());

		if (it != m_components.end())
			return (T*)it->second;
		else
			return nullptr;
	}

	template <class T>
	std::vector<T*> GetComponents(ComponentType type) {
		std::vector<T*> found;
		auto range = m_components.equal_range(type);
		for (auto it = range.first; it != range.second; ++it) {
			found.push_back(it->second);
		}

		return found;
	}

private:
	//std::vector<Component> m_components;
	std::string m_name;
	std::unordered_multimap<ComponentType, Component *> m_components;
};