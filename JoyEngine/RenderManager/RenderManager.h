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

#include "IJoyGraphicsContext.h"

#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/GFXResource.h"

#include "Components/MeshRenderer.h"
#include "Components/MeshRendererTypes.h"
#include "RenderManager/VulkanAllocator.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/VulkanUtils.h"

#include "RenderObject.h"
#include "Swapchain.h"

#include "Utils/FileUtils.h"

namespace JoyEngine {
    class RenderObject;

    class RenderManager {
    public:
        RenderManager() = default;

        RenderManager(IJoyGraphicsContext *const graphicsContext, ResourceManager *const resourceManager);

        ~RenderManager();

        static RenderManager *GetInstance() noexcept { return m_instance; }

        void Init();

        void Start() {}

        void Stop() {}

        void Update();

        void DrawFrame();

        const int MAX_FRAMES_IN_FLIGHT = 2;

        void CreateRenderPass();

        uint32_t RegisterMeshRenderer(MeshRenderer *meshRenderer);

        void UnregisterMeshRenderer(uint32_t);

        void CreateDepthResources();

        void CreateFramebuffers();

        void CreateCommandBuffers();

        void WriteCommandBuffers();

        void ResetCommandBuffers();

        void CreateSyncObjects();

    private:
        static RenderManager *m_instance;

        IJoyGraphicsContext *const m_graphicsContext;
        ResourceManager *const m_resourceManager;
        const VkAllocationCallbacks *m_allocator;
        std::unique_ptr<Swapchain> m_swapchain;
        VkRenderPass m_renderPass;

        uint32_t m_renderObjectIndex = 0;
        std::map<uint32_t, std::unique_ptr<RenderObject>> m_renderObjects;
        std::unique_ptr<GFXTexture> m_depthTexture;
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
