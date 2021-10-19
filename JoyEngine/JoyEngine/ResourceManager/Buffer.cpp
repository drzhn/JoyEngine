#include "Buffer.h"

#include "JoyContext.h"
#include "Utils/Assert.h"
#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	std::string ParseVkResult(VkResult res);
	uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	BufferMappedPtr::BufferMappedPtr(VkDeviceMemory bufferMemory, VkDeviceSize offset, VkDeviceSize size):
		m_bufferMemory(bufferMemory)
	{
		vkMapMemory(JoyContext::Graphics->GetDevice(), m_bufferMemory, offset, size, 0, &m_bufferPtr);
	}

	BufferMappedPtr::~BufferMappedPtr()
	{
		vkUnmapMemory(JoyContext::Graphics->GetDevice(), m_bufferMemory);
	}

	void* BufferMappedPtr::GetMappedPtr() const noexcept
	{
		return m_bufferPtr;
	}

	Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage,
	               VkMemoryPropertyFlags properties):
		m_size(size),
		m_usage(usage),
		m_properties(properties)
	{
		const auto logicalDevice = JoyContext::Graphics->GetDevice();
		const auto allocator = JoyContext::Graphics->GetAllocationCallbacks();

		VkBufferCreateInfo bufferInfo{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			m_size,
			m_usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr
		};

		VkResult res = vkCreateBuffer(logicalDevice, &bufferInfo, allocator, &m_buffer);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, m_buffer, &memRequirements);

		JoyContext::Memory->AllocateMemory(memRequirements, m_properties, m_bufferMemory);

		res = vkBindBufferMemory(logicalDevice, m_buffer, m_bufferMemory, 0);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	std::unique_ptr<BufferMappedPtr> Buffer::GetMappedPtr(VkDeviceSize offset, VkDeviceSize size) const
	{
		ASSERT(m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		ASSERT(size <= m_size);

		std::unique_ptr<BufferMappedPtr> ptr = std::make_unique<BufferMappedPtr>(m_bufferMemory, offset, size);
		return std::move(ptr);
	}

	void Buffer::LoadDataAsync(std::ifstream& stream, uint64_t offset, const std::function<void()>& callback)
	{
		ASSERT(m_properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_onLoadedExternalCallback = callback;
		JoyContext::Memory->LoadDataToBufferAsync(stream, offset, m_size, m_buffer, m_onLoadedInternalCallback);
	}

	Buffer::~Buffer()
	{
		vkDestroyBuffer(JoyContext::Graphics->GetDevice(), m_buffer, JoyContext::Graphics->GetAllocationCallbacks());
		vkFreeMemory(JoyContext::Graphics->GetDevice(), m_bufferMemory,
		             JoyContext::Graphics->GetAllocationCallbacks());
	}

	VkBuffer Buffer::GetBuffer() const noexcept
	{
		return m_buffer;
	}
}
