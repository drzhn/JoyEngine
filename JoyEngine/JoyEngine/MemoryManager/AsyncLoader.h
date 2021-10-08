#ifndef ASYNC_LOADER_H
#define ASYNC_LOADER_H

#include <thread>
#include <atomic>
#include <list>

#include <vulkan/vulkan.h>


namespace JoyEngine
{
	class LoadCommand
	{
	private:
		uint32_t m_loadSize;
	};

	class AsyncLoader
	{
	public:
		AsyncLoader() = delete;

		AsyncLoader(VkQueue transferQueue);
		~AsyncLoader();
		void Update();
	private:
		void WorkLoop();
	private:

		static constexpr uint32_t m_workerSize = 2;

		VkQueue m_transferQueue = VK_NULL_HANDLE;
		std::atomic<bool> m_shouldStopWork = false;
		std::list<LoadCommand> m_commandsQueue;

		std::thread m_workers[m_workerSize];
	};
}
#endif //ASYNC_LOADER_H
