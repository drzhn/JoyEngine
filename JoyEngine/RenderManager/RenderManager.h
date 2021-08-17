#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "windows.h"

#include <vector>
#include <array>
#include <set>
#include <chrono>
#include <map>
#include <vulkan/vulkan.h>

#include "Components/MeshRenderer.h"
#include "Components/MeshRendererTypes.h"
#include "RenderManager/VulkanAllocator.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/VulkanUtils.h"
#include "RenderObject.h"

#include "Utils/FileUtils.h"

namespace JoyEngine {

    class IJoyGraphicsContext {
    public :
        [[nodiscard]] virtual HINSTANCE GetHINSTANCE() const noexcept = 0;

        [[nodiscard]] virtual HWND GetHWND() const noexcept = 0;

        [[nodiscard]] virtual Allocator *GetAllocator() const noexcept = 0;

        [[nodiscard]] virtual VkInstance GetVkInstance() const noexcept = 0;

        [[nodiscard]] virtual VkPhysicalDevice GetVkPhysicalDevice() const noexcept = 0;

        [[nodiscard]] virtual VkDevice GetVkDevice() const noexcept = 0;

        [[nodiscard]] virtual VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() const noexcept = 0;

        [[nodiscard]] virtual VkSurfaceKHR GetVkSurfaceKHR() const noexcept = 0;

        [[nodiscard]] virtual VkQueue GetGraphicsVkQueue() const noexcept = 0;

        [[nodiscard]] virtual VkQueue GetPresentVkQueue() const noexcept = 0;

        [[nodiscard]] virtual VkCommandPool GetVkCommandPool() const noexcept = 0;
    };

    class RenderManager {
    public:
        RenderManager() = default;

        RenderManager(const IJoyGraphicsContext &graphicsContext);

        ~RenderManager() {

        }

        static RenderManager *GetInstance() noexcept { return m_instance; }

        void Init() {}

        void Start() {}

        void Stop() {}

        const int MAX_FRAMES_IN_FLIGHT = 2;

        void CreateSwapChain();

        void CreateImageViews();

        void CreateRenderPass();

        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        uint32_t RegisterMeshRenderer(MeshRenderer *meshRenderer);

        void UnregisterMeshRenderer(uint32_t);

    private:
        static RenderManager *m_instance;
        const IJoyGraphicsContext &m_graphicsContext;
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;

        VkRenderPass renderPass;

        uint32_t m_renderObjectIndex = 0;
        std::map<uint32_t, RenderObject *> m_renderObjects;
//        VkDescriptorSetLayout descriptorSetLayout;
//        VkPipelineLayout pipelineLayout;
//        VkPipeline graphicsPipeline;
//
//        std::vector<VkFramebuffer> swapChainFramebuffers;
//        VkCommandPool commandPool;
//        std::vector<VkCommandBuffer> commandBuffers;
//
//        std::vector<VkSemaphore> imageAvailableSemaphores;
//        std::vector<VkSemaphore> renderFinishedSemaphores;
//        std::vector<VkFence> inFlightFences;
//        std::vector<VkFence> imagesInFlight;
//        size_t currentFrame = 0;
//
//        bool framebufferResized = false;
//
//        std::vector<Vertex> vertices;
//        std::vector<uint32_t> indices;
//
//        VkBuffer vertexBuffer;
//        VkDeviceMemory vertexBufferMemory;
//
//        VkBuffer indexBuffer;
//        VkDeviceMemory indexBufferMemory;
//
//        std::vector<VkBuffer> uniformBuffers;
//        std::vector<VkDeviceMemory> uniformBuffersMemory;
//
//        VkDescriptorPool descriptorPool;
//        std::vector<VkDescriptorSet> descriptorSets;
//
//        VkImage textureImage;
//        VkDeviceMemory textureImageMemory;
//        VkImageView textureImageView;
//        VkSampler textureSampler;
//
//        VkImage depthImage;
//        VkDeviceMemory depthImageMemory;
//        VkImageView depthImageView;

    };


}

#endif //RENDER_MANAGER_H
