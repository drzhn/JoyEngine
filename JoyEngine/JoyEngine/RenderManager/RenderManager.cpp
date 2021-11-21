#include "RenderManager.h"

#include <chrono>
#include <memory>

#include "JoyContext.h"

#include "MemoryManager/MemoryManager.h"
#include "RenderManager/VulkanUtils.h"

#define GLM_FORCE_RADIANS
#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace JoyEngine
{
	RenderManager::RenderManager()
	{
	}

	void RenderManager::Stop()
	{
		vkQueueWaitIdle(JoyContext::Graphics->GetPresentQueue());
		vkDeviceWaitIdle(JoyContext::Graphics->GetDevice());
	}

	RenderManager::~RenderManager()
	{
		m_depthAttachment = nullptr;
		m_normalAttachment = nullptr;
		m_positionAttachment = nullptr;

		for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(JoyContext::Graphics->GetDevice(), m_swapChainFramebuffers[i],
			                     JoyContext::Graphics->GetAllocationCallbacks());
		}

		vkFreeCommandBuffers(JoyContext::Graphics->GetDevice(),
		                     JoyContext::Graphics->GetCommandPool(),
		                     static_cast<uint32_t>(commandBuffers.size()),
		                     commandBuffers.data());

		m_renderPass = nullptr;

		m_swapchain = nullptr;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(JoyContext::Graphics->GetDevice(), m_renderFinishedSemaphores[i],
			                   JoyContext::Graphics->GetAllocationCallbacks());
			vkDestroySemaphore(JoyContext::Graphics->GetDevice(), m_imageAvailableSemaphores[i],
			                   JoyContext::Graphics->GetAllocationCallbacks());
			vkDestroyFence(JoyContext::Graphics->GetDevice(), m_inFlightFences[i],
			               JoyContext::Graphics->GetAllocationCallbacks());
		}
	}

	void RenderManager::Init()
	{
		m_swapchain = std::make_unique<Swapchain>();

		CreateRenderPass();
		CreateFramebuffers();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void RenderManager::CreateRenderPass()
	{
		const VkFormat depthFormat = findDepthFormat(JoyContext::Graphics->GetPhysicalDevice());
		const VkFormat colorFormat = m_swapchain->GetSwapChainImageFormat();
		const VkFormat positionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		const VkFormat normalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

		m_depthAttachment = std::make_unique<Texture>(
			m_swapchain->GetWidth(),
			m_swapchain->GetHeight(),
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT
		);

		m_positionAttachment = std::make_unique<Texture>(
			m_swapchain->GetWidth(),
			m_swapchain->GetHeight(),
			positionFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		m_normalAttachment = std::make_unique<Texture>(
			m_swapchain->GetWidth(),
			m_swapchain->GetHeight(),
			normalFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		const VkAttachmentDescription colorAttachmentDescription = {
			0,
			colorFormat,
			VK_SAMPLE_COUNT_1_BIT, // TODO later
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		const VkAttachmentDescription positionGBufferAttachmentDescription = {
			0,
			positionFormat,
			VK_SAMPLE_COUNT_1_BIT, // TODO later
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		const VkAttachmentDescription normalGBufferAttachmentDescription = {
			0,
			normalFormat,
			VK_SAMPLE_COUNT_1_BIT, // TODO later
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		const VkAttachmentDescription depthAttachmentDescription = {
			0,
			depthFormat,
			VK_SAMPLE_COUNT_1_BIT, // TODO later
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		//const VkAttachmentDescription positionAttachmentDescription = {
		//	0,
		//	VK_FORMAT_R16G16B16_SFLOAT,
		//	VK_SAMPLE_COUNT_1_BIT, // TODO later
		//	VK_ATTACHMENT_LOAD_OP_CLEAR,
		//	VK_ATTACHMENT_STORE_OP_STORE,
		//	VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		//	VK_ATTACHMENT_STORE_OP_DONT_CARE,
		//	VK_IMAGE_LAYOUT_UNDEFINED,
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		//};
		//
		//const VkAttachmentDescription normalAttachmentDescription = {
		//	0,
		//	VK_FORMAT_R16G16B16_SFLOAT,
		//	VK_SAMPLE_COUNT_1_BIT, // TODO later
		//	VK_ATTACHMENT_LOAD_OP_CLEAR,
		//	VK_ATTACHMENT_STORE_OP_STORE,
		//	VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		//	VK_ATTACHMENT_STORE_OP_DONT_CARE,
		//	VK_IMAGE_LAYOUT_UNDEFINED,
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		//};

		VkAttachmentDescription attachments[] = {
			colorAttachmentDescription,
			positionGBufferAttachmentDescription,
			normalGBufferAttachmentDescription,
			depthAttachmentDescription
		};

		constexpr VkAttachmentReference colorAttachmentRef = {
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		constexpr VkAttachmentReference positionAttachmentRef = {
			1,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};
		constexpr VkAttachmentReference normalAttachmentRef = {
			2,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};
		constexpr VkAttachmentReference depthAttachmentRef = {
			3,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		constexpr VkAttachmentReference gbufferColorAttachmentRefs[] = {
			positionAttachmentRef,
			normalAttachmentRef
		};


		const VkSubpassDescription subpasses[] = {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				0,
				nullptr,
				2,
				gbufferColorAttachmentRefs,
				nullptr,
				&depthAttachmentRef,
				0,
				nullptr,
			},
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				0,
				nullptr,
				1,
				&colorAttachmentRef,
				nullptr,
				&depthAttachmentRef,
				0,
				nullptr,
			}
		};

		VkSubpassDependency dependencies[] =
		{
			{
				VK_SUBPASS_EXTERNAL,
				0,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				0
			},
			{
				0,
				1,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				0
			},
			{
				1,
				VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				0
			}
		};


		m_renderPass = std::make_unique<RenderPass>(
			4,
			attachments,
			2,
			subpasses,
			0,
			nullptr);

		//m_gBufferWriteSharedMaterial = std::make_unique<SharedMaterial>(m_gBufferWriteSharedMaterialGuid);
	}

	void RenderManager::CreateFramebuffers()
	{
		m_swapChainFramebuffers.resize(m_swapchain->GetSwapchainImageCount());
		for (size_t i = 0; i < m_swapchain->GetSwapchainImageCount(); i++)
		{
			VkImageView attachments[] = {
				m_swapchain->GetSwapChainImageViews()[i],
				m_positionAttachment->GetImageView(),
				m_normalAttachment->GetImageView(),
				m_depthAttachment->GetImageView()
			};

			VkFramebufferCreateInfo framebufferInfo{
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				m_renderPass->GetRenderPass(),
				4,
				attachments,
				m_swapchain->GetWidth(),
				m_swapchain->GetHeight(),
				1
			};

			const VkResult res = vkCreateFramebuffer(
				JoyContext::Graphics->GetDevice(),
				&framebufferInfo,
				JoyContext::Graphics->GetAllocationCallbacks(),
				&m_swapChainFramebuffers[i]);

			ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
		}
	}

	void RenderManager::RegisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		m_meshRenderers.insert(meshRenderer);
	}

	void RenderManager::UnregisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		if (m_meshRenderers.find(meshRenderer) == m_meshRenderers.end())
		{
			ASSERT(false);
		}
		m_meshRenderers.erase(meshRenderer);
	}

	void RenderManager::RegisterCamera(Camera* camera)
	{
		m_currentCamera = camera;
	}

	void RenderManager::UnregisterCamera(Camera* camera)
	{
		ASSERT(m_currentCamera == camera);
		m_currentCamera = nullptr;
	}

	void RenderManager::CreateCommandBuffers()
	{
		commandBuffers.resize(m_swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = JoyContext::Graphics->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(JoyContext::Graphics->GetDevice(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void RenderManager::WriteCommandBuffers(uint32_t imageIndex) const
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass->GetRenderPass();
		renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = {m_swapchain->GetWidth(), m_swapchain->GetHeight()};

		VkClearValue clearValues[2];
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		ASSERT(m_currentCamera != nullptr);

		for (auto const& mr : m_meshRenderers)
		{
			if (!mr->IsReady()) continue;

			VkBuffer vertexBuffers[] = {
				mr->GetMesh()->GetVertexBuffer()
			};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(
				commandBuffers[imageIndex],
				0,
				1,
				vertexBuffers,
				offsets);

			vkCmdBindIndexBuffer(
				commandBuffers[imageIndex],
				mr->GetMesh()->GetIndexBuffer(),
				0,
				VK_INDEX_TYPE_UINT32);

			vkCmdBindPipeline(
				commandBuffers[imageIndex],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				mr->GetMaterial()->GetSharedMaterial()->GetPipeline());

			auto sets = mr->GetMaterial()->GetDescriptorSets();
			vkCmdBindDescriptorSets(
				commandBuffers[imageIndex],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				mr->GetMaterial()->GetSharedMaterial()->GetPipelineLayout(),
				0,
				1,
				&sets[imageIndex],
				0, nullptr);

			MVP mvp{};
			mvp.model = mr->GetTransform()->GetModelMatrix();
			mvp.view = m_currentCamera->GetViewMatrix();
			mvp.proj = m_currentCamera->GetProjMatrix();

			vkCmdPushConstants(
				commandBuffers[imageIndex],
				mr->GetMaterial()->GetSharedMaterial()->GetPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(MVP),
				&mvp);

			vkCmdDrawIndexed(
				commandBuffers[imageIndex],
				static_cast<uint32_t>(mr->GetMesh()->GetIndexSize()),
				1,
				0,
				0,
				0);
		}

		vkCmdEndRenderPass(commandBuffers[imageIndex]);

		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void RenderManager::ResetCommandBuffers(uint32_t imageIndex)
	{
		vkResetCommandBuffer(commandBuffers[imageIndex], 0);
	}

	void RenderManager::CreateSyncObjects()
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(m_swapchain->GetSwapchainImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(JoyContext::Graphics->GetDevice(), &semaphoreInfo,
			                      JoyContext::Graphics->GetAllocationCallbacks(),
			                      &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(JoyContext::Graphics->GetDevice(), &semaphoreInfo,
				                  JoyContext::Graphics->GetAllocationCallbacks(),
				                  &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(JoyContext::Graphics->GetDevice(), &fenceInfo,
				              JoyContext::Graphics->GetAllocationCallbacks(), &m_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void RenderManager::Update()
	{
		DrawFrame();
	}

	void RenderManager::DrawFrame()
	{
		vkWaitForFences(JoyContext::Graphics->GetDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(JoyContext::Graphics->GetDevice(),
		                                        m_swapchain->GetSwapChain(),
		                                        UINT64_MAX,
		                                        m_imageAvailableSemaphores[currentFrame],
		                                        VK_NULL_HANDLE,
		                                        &imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(JoyContext::Graphics->GetDevice(),
			                1,
			                &m_imagesInFlight[imageIndex],
			                VK_TRUE,
			                UINT64_MAX);
			ResetCommandBuffers(imageIndex);
		}
		WriteCommandBuffers(imageIndex);

		// Mark the image as now being in use by this frame
		m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];
		vkResetFences(JoyContext::Graphics->GetDevice(), 1, &m_inFlightFences[currentFrame]);

		VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};

		VkSubmitInfo submitInfo{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			1,
			waitSemaphores,
			waitStages,
			1,
			&commandBuffers[imageIndex],
			1,
			signalSemaphores
		};

		if (vkQueueSubmit(JoyContext::Graphics->GetGraphicsQueue(), 1, &submitInfo,
		                  m_inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkSwapchainKHR swapChains[] = {m_swapchain->GetSwapChain()};
		VkPresentInfoKHR presentInfo{
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			nullptr,
			1,
			signalSemaphores,
			1,
			swapChains,
			&imageIndex,
			nullptr
		};
		result = vkQueuePresentKHR(JoyContext::Graphics->GetPresentQueue(), &presentInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	Swapchain* RenderManager::GetSwapchain() const noexcept { return m_swapchain.get(); }

	VkRenderPass RenderManager::GetMainRenderPass() const noexcept { return m_renderPass->GetRenderPass(); }

	float RenderManager::GetAspect() const noexcept
	{
		ASSERT(m_swapchain != nullptr);
		return static_cast<float>(m_swapchain->GetWidth()) /
			static_cast<float>(m_swapchain->GetHeight());
	}

	Texture* RenderManager::GetGBufferPositionTexture() const noexcept
	{
		return m_positionAttachment.get();
	}

	Texture* RenderManager::GetGBufferNormalTexture() const noexcept
	{
		return m_normalAttachment.get();
	}
}
