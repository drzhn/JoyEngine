#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#include <vulkan/vulkan_core.h>

#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	class Attachment
	{
	public:
		Attachment(
			VkFormat format,
			VkAttachmentLoadOp loadOp,
			VkAttachmentStoreOp storeOp,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout
		);

		Attachment(
			VkFormat format,
			VkAttachmentLoadOp loadOp,
			VkAttachmentStoreOp storeOp,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout,
			uint32_t width,
			uint32_t height,
			VkImageUsageFlags usage,
			VkImageAspectFlags aspectFlags
		);
		~Attachment();

		[[nodiscard]] VkAttachmentDescription GetAttachmentDesc() const;
		[[nodiscard]] VkImageView GetImageView() const;
	private:
		std::unique_ptr<Texture> m_texture;
		VkFormat m_format;
		VkAttachmentLoadOp m_loadOp;
		VkAttachmentStoreOp m_storeOp;
		VkImageLayout m_initialLayout;
		VkImageLayout m_finalLayout;
	};
}
#endif
