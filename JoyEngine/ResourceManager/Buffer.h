#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>

#include "Common/Resource.h"

namespace JoyEngine
{
	class Buffer : public Resource
	{
	public:
		Buffer() = delete;

		explicit Buffer(VkDeviceSize size, VkBufferUsageFlags usage,
		                VkMemoryPropertyFlags properties);

		~Buffer() final;

		void SetDeviceLocalData(void const* data, VkDeviceSize size) const;

		[[nodiscard]] VkBuffer GetBuffer() const noexcept;
	private:
		VkDeviceSize m_size = 0;
		VkBufferUsageFlags m_usage = 0;
		VkMemoryPropertyFlags m_properties = 0;

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
	};
}


#endif // BUFFER_H
