#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>

namespace JoyEngine
{
	class RenderPass
	{
	public:
		RenderPass() = delete;
		explicit RenderPass(
			uint32_t attachmentsCount, const VkAttachmentDescription* attachments,
			uint32_t subpassCount, const VkSubpassDescription* subpasses,
			uint32_t dependencyCount, const VkSubpassDependency* dependencies);
		~RenderPass();
		[[nodiscard]] VkRenderPass GetRenderPass() const noexcept;
	private:
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
	};
}
#endif	// RENDERPASS_H 
