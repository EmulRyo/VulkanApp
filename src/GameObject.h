#pragma once

#include <string>
#include <vector>
#include "Transform.h"

class Model;

class GameObject {
public:
	GameObject(const std::string &name) {
		m_name = name;
		AddComponent<Transform>();
	};
	GameObject(const GameObject&) = default;
	virtual ~GameObject() {
		Dispose();
	};

	void Dispose() {
		if (m_components.size() > 0) {
			for (auto it = m_components.begin(); it != m_components.end(); ++it) {
				delete it->second;
			}
			m_components.clear();
		}
	}

	void AddComponent(Component* component) { m_components.emplace(component->GetType(), component); }

	template <class T, class... Args>
	T* AddComponent(Args&&... args) { 
		T* t = new T(std::forward<Args>(args)...);
		m_components.emplace(t->GetType(), t);
		return t;
	}
	
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
	
	int DelComponents(ComponentType type) {
		return (int)m_components.erase(type);
	}
	
private:
	//std::vector<Component> m_components;
	std::string m_name;
	std::unordered_multimap<ComponentType, Component *> m_components;
};