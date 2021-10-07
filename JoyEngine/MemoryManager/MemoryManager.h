#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vulkan/vulkan.h>

namespace JoyEngine
{
	class JoyEngine;

	class MemoryManager
	{
	public:
		MemoryManager() = default;

		void Init()
		{
		}

		void Start()
		{
		}

		void Stop()
		{
		}

		void AllocateMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties,
		                    VkDeviceMemory& out_imageMemory);

		void LoadDataToBuffer(
			void* data,
			size_t bufferSize, VkBuffer gpuBuffer);

		void LoadDataToImage(
			const unsigned char* data,
			uint32_t width,
			uint32_t height,
			VkImage gpuImage);

	private:
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkCommandBuffer BeginSingleTimeCommands();

		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	};
}

#endif
