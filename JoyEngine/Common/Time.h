#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace JoyEngine {

	class Time
	{
	public :
		static void Init() noexcept;
		static void Update() noexcept;
		static float GetDeltaTime() noexcept;
		static float GetTime() noexcept;
		static float GetFrameCount() noexcept;
	private:
		static float m_deltaTime;
		static float m_time;
		static uint32_t m_frameCount;
		static std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
		static std::chrono::time_point<std::chrono::high_resolution_clock> m_prevUpdateTime;
		static std::chrono::time_point<std::chrono::high_resolution_clock> m_curUpdateTime;
	};
}
#endif //TIME_H

