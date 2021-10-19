#include "AsyncLoader.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"
namespace JoyEngine
{
	LoadCommand::LoadCommand(std::ifstream& binaryStream, uint32_t streamOffset,
	                         const std::function<void()>& onLoadedCallback):
		m_binaryStream(binaryStream),
		m_streamOffset(streamOffset),
		m_onLoadedCallback(onLoadedCallback)
	{
	}

	void LoadCommand::CreateStagingBuffer()
	{
		ASSERT(m_loadSize != 0);
		ASSERT(m_binaryStream.is_open());

		m_stagingBuffer = std::make_unique<Buffer>(
			m_loadSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		const std::unique_ptr<BufferMappedPtr> ptr = m_stagingBuffer->GetMappedPtr(0, m_loadSize);

		m_binaryStream.seekg(m_streamOffset);
		m_binaryStream.read(static_cast<char*>(ptr->GetMappedPtr()), m_loadSize);
	}

	BufferLoadCommand::BufferLoadCommand(VkBuffer gpuBuffer, std::ifstream& binaryStream, uint32_t streamOffset,
	                                     VkDeviceSize loadSize,
	                                     const std::function<void()>& onLoadedCallback) :
		LoadCommand(binaryStream, streamOffset, onLoadedCallback),
		m_gpuBuffer(gpuBuffer)
	{
		m_loadSize = loadSize;
	}

	void BufferLoadCommand::WriteCommandBuffer(VkCommandBuffer& commandBuffer)
	{
		const VkBufferCopy copyRegion{
			0,
			0,
			m_loadSize
		};
		vkCmdCopyBuffer(commandBuffer, m_stagingBuffer->GetBuffer(), m_gpuBuffer, 1, &copyRegion);
	}

	ImageLoadCommand::ImageLoadCommand(VkImage gpuImage, std::ifstream& binaryStream, uint32_t streamOffset,
	                                   uint32_t width,
	                                   uint32_t height, const std::function<void()>& onLoadedCallback):
		LoadCommand(binaryStream, streamOffset, onLoadedCallback),
		m_gpuImage(gpuImage),
		m_width(width),
		m_height(height)
	{
		m_loadSize = width * height * 4;
	}

	void ImageLoadCommand::WriteCommandBuffer(VkCommandBuffer& commandBuffer)
	{
		WriteTransitionImageLayout(
			commandBuffer,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

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
			m_width,
			m_height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			m_stagingBuffer->GetBuffer(),
			m_gpuImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region);

		WriteTransitionImageLayout(
			commandBuffer,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	void ImageLoadCommand::WriteTransitionImageLayout(const VkCommandBuffer& commandBuffer, VkFormat format,
	                                                  const VkImageLayout oldLayout,
	                                                  const VkImageLayout newLayout) const
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_gpuImage;
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
	}

	AsyncLoader::AsyncLoader(VkQueue transferQueue): m_transferQueue(transferQueue)
	{
		//m_worker = std::thread(&AsyncLoader::WorkLoop, this);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(
			JoyContext::Graphics->GetDevice(),
			&fenceInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_waitFence);


		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = JoyContext::Graphics->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(JoyContext::Graphics->GetDevice(), &allocInfo, &m_commandBuffer) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	AsyncLoader::~AsyncLoader()
	{
		vkFreeCommandBuffers(JoyContext::Graphics->GetDevice(),
		                     JoyContext::Graphics->GetCommandPool(),
		                     1,
		                     &m_commandBuffer);
		vkDestroyFence(
			JoyContext::Graphics->GetDevice(),
			m_waitFence,
			JoyContext::Graphics->GetAllocationCallbacks());
		//m_shouldStopWork.store(true, std::memory_order_release);
		//if (m_worker.joinable())
		//{
		//	m_worker.join();
		//}
	}

	void AsyncLoader::Update()
	{
		vkWaitForFences(JoyContext::Graphics->GetDevice(), 1, &m_waitFence, VK_TRUE, UINT64_MAX);
		for (const auto& command : m_processingCommands)
		{
			command->OnCompleted();
		}
		m_processingCommands.clear();
		vkResetCommandBuffer(m_commandBuffer, 0);
		vkResetFences(JoyContext::Graphics->GetDevice(), 1, &m_waitFence);


		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		while (!m_commandsQueue.empty())
		{
			std::unique_ptr<LoadCommand> loadCommand = std::move(m_commandsQueue.front());
			m_commandsQueue.pop_front();
			loadCommand->CreateStagingBuffer();
			loadCommand->WriteCommandBuffer(m_commandBuffer);
			m_processingCommands.push_back(std::move(loadCommand));
		}

		if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		VkSubmitInfo submitInfo{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0,
			nullptr,
			nullptr,
			1,
			&m_commandBuffer,
			0,
			nullptr
		};

		if (vkQueueSubmit(
			JoyContext::Graphics->GetGraphicsQueue(),
			1,
			&submitInfo,
			m_waitFence) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}

	void AsyncLoader::LoadDataToBuffer(
		std::ifstream& stream, uint64_t offset, uint64_t bufferSize, VkBuffer gpuBuffer,
		const std::function<void()>& callback)
	{
		m_commandsQueue.push_back(
			std::make_unique<BufferLoadCommand>(gpuBuffer, stream, offset, bufferSize, callback));
	}

	void AsyncLoader::LoadDataToImage(
		std::ifstream& stream, uint64_t offset, uint32_t width, uint32_t height,
		VkImage gpuImage, const std::function<void()>& callback)
	{
		m_commandsQueue.push_back(
			std::make_unique<ImageLoadCommand>(gpuImage, stream, offset, width, height, callback));
	}

	//void AsyncLoader::WorkLoop()
	//{
	//	while (true)
	//	{
	//		if (m_shouldStopWork.load(std::memory_order_acquire))
	//		{
	//			break;
	//		}
	//	}
	//}
}
