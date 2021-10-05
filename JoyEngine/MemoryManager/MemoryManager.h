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

		static void CreateBuffer(
			VkPhysicalDevice physicalDevice,
			VkDevice logicalDevice,
			const VkAllocationCallbacks* allocator,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);


		void CreateGPUBuffer(
			void* data,
			size_t stride,
			size_t size,
			VkBuffer& vertexBuffer,
			VkDeviceMemory& vertexBufferMemory,
			VkBufferUsageFlagBits usageFlag);

		void DestroyBuffer(
			VkBuffer vertexBuffer,
			VkDeviceMemory vertexBufferMemory);

		void LoadDataToImage(
			const unsigned char* data,
			uint32_t width,
			uint32_t height,
			VkImage textureImage);

		void CreateShaderModule(const uint32_t* code, size_t codeSize, VkShaderModule& shaderModule);

		void DestroyShaderModule(VkShaderModule shaderModule);

	private:
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkCommandBuffer BeginSingleTimeCommands();

		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	};
}

#endif
