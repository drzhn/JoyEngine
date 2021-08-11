#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "windows.h"

#include <vector>
#include <array>
#include <set>
#include <chrono>

#include <vulkan/vulkan.h>

#include "RenderManager/VulkanAllocator.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/VulkanUtils.h"

#include "Utils/FileUtils.h"

namespace JoyEngine {
    class RenderManager {
    public:
        RenderManager() = default;

        RenderManager(HINSTANCE instance, HWND windowHandle);

        ~RenderManager() {

        }

        void Init() {}

        void Start() {}

        void Stop() {}

        const char *MODEL_PATH = "../models/viking_room.obj";
        const char *TEXTURE_PATH = "../textures/viking_room.png";

        const int MAX_FRAMES_IN_FLIGHT = 2;

        void CreateInstance();

        void SetupDebugMessenger();

        void CreateSurface();

        void PickPhysicalDevice();

        void CreateLogicalDevice();

        void CreateSwapChain();

        void CreateImageViews();

        void CreateRenderPass();

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    private:
        const std::vector<const char *> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        const char *validationLayerName = "VK_LAYER_KHRONOS_validation";

        HINSTANCE m_instance;
        HWND m_windowHandle;

        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;

        VkRenderPass renderPass;
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

        Allocator *allocator_;
    };


}

#endif //RENDER_MANAGER_H
