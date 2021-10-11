#ifndef TEXTURE_H
#define TEXTURE_H

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
		void InitializeTexture(std::ifstream& stream, uint64_t offset);

		[[nodiscard]] VkImage& GetImage() noexcept { return m_textureImage; }

		[[nodiscard]] VkDeviceMemory& GetDeviceMemory() noexcept { return m_textureImageMemory; }

		[[nodiscard]] VkImageView& GetImageView() noexcept { return m_textureImageView; }

		[[nodiscard]] VkSampler& GetSampler() noexcept { return m_textureSampler; }

	private:
		void CreateImage();
		void CreateImageView();
		void CreateImageSampler();

	private :
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		VkFormat m_format = VK_FORMAT_UNDEFINED;
		VkImageTiling m_tiling = VK_IMAGE_TILING_MAX_ENUM;
		VkImageUsageFlags m_usageFlags = 0;
		VkMemoryPropertyFlags m_propertiesFlags = 0;
		VkImageAspectFlags m_aspectFlags = 0;


		VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
		VkImage m_textureImage = VK_NULL_HANDLE;
		VkImageView m_textureImageView = VK_NULL_HANDLE;
		VkSampler m_textureSampler = VK_NULL_HANDLE;
	};
}


#endif //TEXTURE_H
