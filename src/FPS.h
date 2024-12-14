#pragma once

class FPS
{
public:
	FPS(float measureTime) {
		m_measureTime = measureTime;
		m_numSamples = 0;
		m_sumSamples = 0;
		m_frameTime = 0.01667f;
	}

	void Update(float deltaTime) {
		m_sumSamples += deltaTime;
		m_numSamples++;

		if (m_sumSamples >= m_measureTime) {
			m_frameTime = m_sumSamples / m_numSamples;
			m_sumSamples = 0;
			m_numSamples = 0;
		}
	};

	int GetFPS() const {
		return (int)(roundf(1.0f / m_frameTime));
	}

	float GetFrametime() const {
		return m_frameTime;
	}

private:
	float m_measureTime;
	int m_numSamples;
	float m_sumSamples;
	float m_frameTime;
};

