#include "AsyncLoader.h"

namespace JoyEngine
{
	AsyncLoader::AsyncLoader(VkQueue transferQueue): m_transferQueue(transferQueue)
	{
		m_worker = std::thread(&AsyncLoader::WorkLoop, this);
	}

	AsyncLoader::~AsyncLoader()
	{
		m_shouldStopWork.store(true, std::memory_order_release);
		if (m_worker.joinable())
		{
			m_worker.join();
		}
	}

	void AsyncLoader::Update()
	{
	}

	void AsyncLoader::WorkLoop()
	{
		while (true)
		{
			if (m_shouldStopWork.load(std::memory_order_acquire))
			{
				break;
			}
		}
	}
}
