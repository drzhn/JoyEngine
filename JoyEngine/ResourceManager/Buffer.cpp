#include "Buffer.h"

#include "JoyContext.h"
#include "Utils/Assert.h"
#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	std::string ParseVkResult(VkResult res);
	uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage,
	               VkMemoryPropertyFlags properties):
		m_size(size),
		m_usage(usage),
		m_properties(properties)
	{
		const auto logicalDevice = JoyContext::Graphics->GetVkDevice();
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

	void Buffer::SetDeviceLocalData(void const* data, VkDeviceSize size) const
	{
		ASSERT(m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		ASSERT(size <= m_size);

		void* bufferPtr;
		vkMapMemory(JoyContext::Graphics->GetVkDevice(), m_bufferMemory, 0, size, 0, &bufferPtr);
		memcpy(bufferPtr, data, size);
		vkUnmapMemory(JoyContext::Graphics->GetVkDevice(), m_bufferMemory);
	}

	Buffer::~Buffer()
	{
		vkDestroyBuffer(JoyContext::Graphics->GetVkDevice(), m_buffer, JoyContext::Graphics->GetAllocationCallbacks());
		vkFreeMemory(JoyContext::Graphics->GetVkDevice(), m_bufferMemory,
		             JoyContext::Graphics->GetAllocationCallbacks());
	}

	VkBuffer Buffer::GetBuffer() const noexcept
	{
		return m_buffer;
	}
}
