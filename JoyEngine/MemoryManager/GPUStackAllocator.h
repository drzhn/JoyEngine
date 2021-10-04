#ifndef GPU_LINEAR_ALLOCATOR_H
#define GPU_LINEAR_ALLOCATOR_H

#include <vulkan/vulkan.h>

namespace JoyEngine
{
	class MemoryChunk
	{
	public:
		MemoryChunk(uint32_t alignment);
		~MemoryChunk();
	private:
		VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
	};
}


#endif //GPU_LINEAR_ALLOCATOR_H
