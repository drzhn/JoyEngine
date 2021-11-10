#include "Attachment.h"

#include "VulkanUtils.h"

namespace JoyEngine
{
	Attachment::Attachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
	                       VkImageLayout initialLayout, VkImageLayout finalLayout):
		m_format(format),
		m_loadOp(loadOp),
		m_storeOp(storeOp),
		m_initialLayout(initialLayout),
		m_finalLayout(finalLayout)

	{
	}

	Attachment::Attachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
	                       VkImageLayout initialLayout, VkImageLayout finalLayout, uint32_t width, uint32_t height,
	                       VkImageUsageFlags usage, VkImageAspectFlags aspectFlags):
		Attachment(format, loadOp, storeOp,
		           initialLayout, finalLayout)
	{
		m_texture = std::make_unique<Texture>(
			width,
			height,
			m_format,
			VK_IMAGE_TILING_OPTIMAL,
			usage,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			aspectFlags);
	}

	Attachment::~Attachment()
	{
	}

	VkAttachmentDescription Attachment::GetAttachmentDesc() const
	{
		const VkAttachmentDescription depthAttachment{
			0,
			m_format,
			VK_SAMPLE_COUNT_1_BIT, // TODO later
			m_loadOp,
			m_storeOp,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			m_initialLayout,
			m_finalLayout
		};

		return depthAttachment;
	}

	VkImageView Attachment::GetImageView() const
	{
		return m_texture != nullptr ? m_texture->GetImageView() : VK_NULL_HANDLE;
	}
}
