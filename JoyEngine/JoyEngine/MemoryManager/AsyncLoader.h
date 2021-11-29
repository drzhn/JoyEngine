#ifndef ASYNC_LOADER_H
#define ASYNC_LOADER_H

#include <thread>
#include <atomic>
#include <list>
#include <fstream>
#include <functional>

#include <vulkan/vulkan.h>

#include "ResourceManager/Buffer.h"


namespace JoyEngine
{
	class LoadCommand
	{
	public:
		LoadCommand() = delete;
		explicit LoadCommand(
			std::ifstream& binaryStream,
			uint32_t streamOffset,
			const std::function<void()>& onLoadedCallback);

		virtual ~LoadCommand() = default;
		void CreateStagingBuffer();
		virtual void WriteCommandBuffer(VkCommandBuffer& commandBuffer) = 0;
		void OnCompleted() const { m_onLoadedCallback(); }
	protected:
		std::unique_ptr<Buffer> m_stagingBuffer;
		std::ifstream& m_binaryStream;
		uint32_t m_streamOffset = 0;
		VkDeviceSize m_loadSize = 0;
		const std::function<void()>& m_onLoadedCallback;
	};

	class BufferLoadCommand : public LoadCommand
	{
	public:
		BufferLoadCommand(
			VkBuffer gpuBuffer,
			std::ifstream& binaryStream,
			uint32_t streamOffset,
			VkDeviceSize loadSize,
			const std::function<void()>& onLoadedCallback);
		void WriteCommandBuffer(VkCommandBuffer& commandBuffer) override;
	private:
		VkBuffer m_gpuBuffer;
	};

	class ImageLoadCommand : public LoadCommand
	{
	public:
		ImageLoadCommand(
			VkImage gpuImage,
			std::ifstream& binaryStream,
			uint32_t streamOffset,
			uint32_t width,
			uint32_t height,
			const std::function<void()>& onLoadedCallback);
		void WriteCommandBuffer(VkCommandBuffer& commandBuffer) override;
	private:
		void WriteTransitionImageLayout(const VkCommandBuffer& commandBuffer, VkFormat format, VkImageLayout oldLayout,
		                                VkImageLayout newLayout) const;
	private:
		uint32_t m_width;
		uint32_t m_height;
		VkImage m_gpuImage;
	};

	class AsyncLoader
	{
	public:
		AsyncLoader() = delete;
		explicit AsyncLoader(VkQueue transferQueue);
		~AsyncLoader();
		void Update();
		void LoadDataToBuffer(std::ifstream& stream, uint32_t offset, uint64_t bufferSize,
		                      VkBuffer gpuBuffer, const std::function<void()>& callback);
		void LoadDataToImage(std::ifstream& stream, uint32_t offset,
			uint32_t width,
			uint32_t height,
			VkImage gpuImage, const std::function<void()>& callback);
	private:
		//void WorkLoop();
	private:
		VkQueue m_transferQueue = VK_NULL_HANDLE;
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
		VkFence m_waitFence = VK_NULL_HANDLE;
		//static constexpr uint32_t m_workerSize = 2;
		//std::thread m_workers[m_workerSize];
		//std::atomic<bool> m_shouldStopWork = false;

		std::list<std::unique_ptr<LoadCommand>> m_commandsQueue;
		std::list<std::unique_ptr<LoadCommand>> m_processingCommands;
	};
}
#endif //ASYNC_LOADER_H
