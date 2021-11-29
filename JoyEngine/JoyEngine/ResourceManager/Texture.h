#ifndef TEXTURE_H
#define TEXTURE_H

#include <functional>
#include <fstream>
#include <vulkan/vulkan.h>

#include "Common/Resource.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class Texture final : public Resource
	{
	public :
		explicit Texture();

		explicit Texture(GUID);

		explicit Texture(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImageAspectFlags aspectFlags);

		~Texture() final;

		void InitializeTexture(const unsigned char* data);
		void InitializeTexture(std::ifstream& stream, uint32_t offset);

		void LoadDataAsync(
			std::ifstream& stream,
			uint64_t offset) const;

		[[nodiscard]] VkImage& GetImage() noexcept { return m_textureImage; }

		[[nodiscard]] VkDeviceMemory& GetDeviceMemory() noexcept { return m_textureImageMemory; }

		[[nodiscard]] VkImageView& GetImageView() noexcept { return m_textureImageView; }

		[[nodiscard]] VkSampler& GetSampler() noexcept { return m_textureSampler; }

		[[nodiscard]] bool IsLoaded() const noexcept override { return m_isLoaded; }

		[[nodiscard]] VkImageSubresourceRange* GetSubresourceRange() noexcept { return &m_subresourceRange; }
	private:
		void CreateImage();
		void CreateImageView();
		void CreateImageSampler();

	private :
		bool m_isLoaded = false;
		std::ifstream m_textureStream;
		std::function<void()> m_onLoadedCallback = [this]()
		{
			if (m_textureStream.is_open())
			{
				m_textureStream.close();
			}
			m_isLoaded = true;
		};

		uint32_t m_width = 0;
		uint32_t m_height = 0;
		VkFormat m_format = VK_FORMAT_UNDEFINED;
		VkImageTiling m_tiling = VK_IMAGE_TILING_MAX_ENUM;
		VkImageUsageFlags m_usageFlags = 0;
		VkMemoryPropertyFlags m_propertiesFlags = 0;
		VkImageAspectFlags m_aspectFlags = 0;
		VkImageSubresourceRange m_subresourceRange;


		VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
		VkImage m_textureImage = VK_NULL_HANDLE;
		VkImageView m_textureImageView = VK_NULL_HANDLE;
		VkSampler m_textureSampler = VK_NULL_HANDLE;
	};
}


#endif //TEXTURE_H
