#include "MemoryManager.h"

#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include "JoyContext.h"


#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/VulkanUtils.h"
#include "ResourceManager/Buffer.h"
//#include "GPUMemoryManager.h"
//#include "Common/Resource.h"
#include "Utils/Assert.h"
//#include "Utils/FileUtils.h"

namespace JoyEngine
{
	void MemoryManager::Init()
	{
		m_dataLoader = std::make_unique<AsyncLoader>(JoyContext::Graphics->GetTransferQueue());
	}

	void MemoryManager::Update()
	{
		m_dataLoader->Update();
	}

	void MemoryManager::AllocateMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties,
	                                   VkDeviceMemory& out_imageMemory)
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			JoyContext::Graphics->GetPhysicalDevice(),
			requirements.memoryTypeBits,
			properties);

		const VkResult res = vkAllocateMemory(
			JoyContext::Graphics->GetDevice(),
			&allocInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&out_imageMemory);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	void MemoryManager::LoadDataToImage(
		std::ifstream& stream,
		uint64_t offset,
		uint32_t width,
		uint32_t height,
		VkImage gpuImage)
	{
		VkDeviceSize imageSize = width * height * 4;

		Buffer stagingBuffer = Buffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, imageSize);
		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), imageSize);

		TransitionImageLayout(
			gpuImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);
		CopyBufferToImage(stagingBuffer.GetBuffer(), gpuImage, width, height);
		TransitionImageLayout(
			gpuImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}


	void MemoryManager::LoadDataToImage(
		const unsigned char* data,
		uint32_t width,
		uint32_t height,
		VkImage gpuImage)
	{
		VkDeviceSize imageSize = width * height * 4;

		Buffer stagingBuffer = Buffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, imageSize);
		memcpy(ptr->GetMappedPtr(), data, imageSize);

		TransitionImageLayout(
			gpuImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);
		CopyBufferToImage(stagingBuffer.GetBuffer(), gpuImage, width, height);
		TransitionImageLayout(
			gpuImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	void MemoryManager::LoadDataToBuffer(
		std::ifstream& stream,
		uint64_t offset,
		uint64_t bufferSize,
		VkBuffer gpuBuffer)
	{
		const Buffer stagingBuffer = Buffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, bufferSize);
		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), bufferSize);
		CopyBuffer(stagingBuffer.GetBuffer(), gpuBuffer, bufferSize);
	}

	void MemoryManager::LoadDataToBufferAsync(std::ifstream& stream, uint32_t offset, uint64_t bufferSize,
	                                          VkBuffer gpuBuffer, const std::function<void()>& callback) const
	{
		m_dataLoader->LoadDataToBuffer(stream, offset, bufferSize, gpuBuffer, callback);
	}

	void MemoryManager::LoadDataToImageAsync(std::ifstream& stream, uint32_t offset,
	                                         uint32_t width,
	                                         uint32_t height,
	                                         VkImage gpuImage, const std::function<void()>& callback) const
	{
		m_dataLoader->LoadDataToImage(stream, offset, width, height, gpuImage, callback);
	}

	void MemoryManager::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void MemoryManager::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(commandBuffer);
	}

	void MemoryManager::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
	                                          VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer MemoryManager::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = JoyContext::Graphics->GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(JoyContext::Graphics->GetDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void MemoryManager::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(JoyContext::Graphics->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(JoyContext::Graphics->GetGraphicsQueue());

		vkFreeCommandBuffers(JoyContext::Graphics->GetDevice(), JoyContext::Graphics->GetCommandPool(), 1,
		                     &commandBuffer);
	}
}
