#ifndef BUFFER_H
#define BUFFER_H

#include <functional>
#include <memory>
#include <vulkan/vulkan.h>

#include "Common/Resource.h"

namespace JoyEngine
{
	class BufferMappedPtr
	{
	public:
		BufferMappedPtr() = delete;
		BufferMappedPtr(VkDeviceMemory bufferMemory, VkDeviceSize offset, VkDeviceSize size);
		~BufferMappedPtr();
		[[nodiscard]] void* GetMappedPtr() const noexcept;
	private:
		void* m_bufferPtr = nullptr;
		VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
	};

	class Buffer final : public Resource
	{
	public:
		Buffer() = delete;

		explicit Buffer(VkDeviceSize size, VkBufferUsageFlags usage,
		                VkMemoryPropertyFlags properties);

		~Buffer() final;

		void LoadDataAsync(
			std::ifstream& stream,
			uint64_t offset, 
			const std::function<void()>& callback);

		[[nodiscard]] std::unique_ptr<BufferMappedPtr> GetMappedPtr(VkDeviceSize offset, VkDeviceSize size) const;

		[[nodiscard]] VkBuffer GetBuffer() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override { return m_isLoaded; }
	private:
		bool m_isLoaded = false;
		std::function<void()> m_onLoadedExternalCallback;
		std::function<void()> m_onLoadedInternalCallback = [this]()
		{
			m_isLoaded = true;
			if (m_onLoadedExternalCallback)
			{
				m_onLoadedExternalCallback();
			}
		};
		VkDeviceSize m_size = 0;
		VkBufferUsageFlags m_usage = 0;
		VkMemoryPropertyFlags m_properties = 0;

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
	};
}


#endif // BUFFER_H
