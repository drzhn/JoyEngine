#include "RenderPass.h"
#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	RenderPass::RenderPass(
		uint32_t attachmentsCount,
		const VkAttachmentDescription* attachments,
		uint32_t subpassCount,
		const VkSubpassDescription* subpasses,
		uint32_t dependencyCount,
		const VkSubpassDependency* dependencies
	)
	{
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentsCount;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = subpassCount;
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = dependencyCount;
		renderPassInfo.pDependencies = dependencies;

		VkResult res = vkCreateRenderPass(
			JoyContext::Graphics->GetDevice(),
			&renderPassInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_renderPass);
		ASSERT(res == VK_SUCCESS);
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(
			JoyContext::Graphics->GetDevice(),
			m_renderPass,
			JoyContext::Graphics->GetAllocationCallbacks());
	}

	VkRenderPass RenderPass::GetRenderPass() const noexcept
	{
		return m_renderPass;
	}
}
