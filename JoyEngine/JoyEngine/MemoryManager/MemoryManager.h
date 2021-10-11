#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <fstream>
#include <vulkan/vulkan.h>
#include <MemoryManager/AsyncLoader.h>


namespace JoyEngine
{
	class JoyEngine;

	class MemoryManager
	{
	public:
		MemoryManager() = default;

		void Init();

		void Update();

		void AllocateMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties,
		                    VkDeviceMemory& out_imageMemory);

		void LoadDataToBuffer(std::ifstream& stream, uint64_t offset, uint64_t bufferSize, VkBuffer gpuBuffer);

		void LoadDataToBufferAsync(
			std::ifstream& stream,
			uint64_t offset,
			uint64_t bufferSize, VkBuffer gpuBuffer, const std::function<void()>& callback) const;

		void LoadDataToImageAsync(
			std::ifstream& stream, uint64_t offset, uint32_t width, uint32_t height,
			VkImage gpuImage,
			const std::function<void()>& callback) const;

		void LoadDataToImage(
			const unsigned char* data,
			uint32_t width,
			uint32_t height,
			VkImage gpuImage);

		void LoadDataToImage(
			std::ifstream& stream,
			uint64_t offset,
			uint32_t width,
			uint32_t height,
			VkImage gpuImage);

	private:
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkCommandBuffer BeginSingleTimeCommands();

		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	private:
		std::unique_ptr<AsyncLoader> m_dataLoader;
	};
}

#endif
