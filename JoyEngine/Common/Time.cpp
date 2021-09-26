#include "Time.h"

namespace JoyEngine {

	float Time::m_deltaTime = 0;
	float Time::m_time = 0;
	uint32_t Time::m_frameCount = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> Time::m_startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> Time::m_prevUpdateTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> Time::m_curUpdateTime;

	void Time::Init() noexcept {
		m_startTime = std::chrono::high_resolution_clock::now();
		m_prevUpdateTime = m_startTime;
	}
	void Time::Update() noexcept {
		m_curUpdateTime = std::chrono::high_resolution_clock::now();
		m_time = std::chrono::duration<float, std::chrono::seconds::period>(m_curUpdateTime - m_startTime).count();
		m_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(m_curUpdateTime - m_prevUpdateTime).count();
		m_prevUpdateTime = m_curUpdateTime;
		m_frameCount++;
	}
	float Time::GetDeltaTime() noexcept {
		return m_deltaTime;
	}
	float Time::GetTime() noexcept {
		return m_time;
	}
	float Time::GetFrameCount() noexcept {
		return m_frameCount;
	}
}
