#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "windows.h"

#include <vector>
#include <array>
#include <set>
#include <chrono>
#include <map>
#include <memory>

#include <vulkan/vulkan.h>

#include "ResourceManager/Texture.h"

#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "RenderManager/Attachment.h"

namespace JoyEngine
{
	class RenderObject;

	class RenderManager
	{
	public:
		RenderManager();

		~RenderManager();

		void Init();

		void Start()
		{
		}

		void Stop();

		void Update();

		void DrawFrame();

		void RegisterMeshRenderer(MeshRenderer* meshRenderer);

		void UnregisterMeshRenderer(MeshRenderer* meshRenderer);

		void RegisterCamera(Camera* camera);

		void UnregisterCamera(Camera* camera);

		[[nodiscard]] Swapchain* GetSwapchain() const noexcept;

		[[nodiscard]] VkRenderPass GetMainRenderPass() const noexcept;

		[[nodiscard]] float GetAspect() const noexcept;
		[[nodiscard]] Texture* GetGBufferPositionTexture() const noexcept;
		[[nodiscard]] Texture* GetGBufferNormalTexture() const noexcept;

		[[nodiscard]] uint32_t GetMainColorSubpassIndex() const noexcept { return 1; }
		[[nodiscard]] uint32_t GetGBufferWriteSubpassIndex() const noexcept { return 0; }
		[[nodiscard]] uint32_t GetMainColorSubpassColorAttachmentsCount() const noexcept { return 1; }
		[[nodiscard]] uint32_t GetGBufferWriteSubpassColorAttachmentsCount() const noexcept { return 2; }

	private:
		void CreateRenderPass();

		void CreateFramebuffers();

		void CreateCommandBuffers();

		void WriteCommandBuffers(uint32_t imageIndex) const;

		void ResetCommandBuffers(uint32_t imageIndex);

		void CreateSyncObjects();

	private:
		const int MAX_FRAMES_IN_FLIGHT = 2;
		//const GUID m_gBufferWriteSharedMaterialGuid = ;
		ResourceHandle<SharedMaterial> m_gBufferWriteSharedMaterial;

		std::unique_ptr<Swapchain> m_swapchain;
		std::unique_ptr<RenderPass> m_renderPass;
		std::unique_ptr<Texture> m_depthAttachment;
		std::unique_ptr<Texture> m_positionAttachment;
		std::unique_ptr<Texture> m_normalAttachment;


		std::set<MeshRenderer*> m_meshRenderers;
		Camera* m_currentCamera;

		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t currentFrame = 0;
	};
}

#endif //RENDER_MANAGER_H
