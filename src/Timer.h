#pragma once

#include <chrono>

class Timer
{
public:
	Timer() {
		Start();
	}

	void Start() {
		m_start = std::chrono::high_resolution_clock::now();
	};

	float Stop() const {
		auto now = std::chrono::high_resolution_clock::now();
		float seconds = std::chrono::duration<float, std::chrono::seconds::period>(now - m_start).count();
		return  seconds;
	};

private:
	std::chrono::high_resolution_clock::time_point m_start;
};

