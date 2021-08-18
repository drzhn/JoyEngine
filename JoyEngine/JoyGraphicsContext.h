#ifndef JOY_GRAPHICS_CONTEXT_H
#define JOY_GRAPHICS_CONTEXT_H

#include "windows.h"
#include <vulkan/vulkan.h>
#include "IJoyGraphicsContext.h"

namespace JoyEngine {
    class JoyGraphicsContext : public IJoyGraphicsContext {
    public:
        JoyGraphicsContext(HINSTANCE instance, HWND windowHandle);

        ~JoyGraphicsContext();

        [[nodiscard]] HINSTANCE GetHINSTANCE() const noexcept override { return m_windowInstance; }

        [[nodiscard]] HWND GetHWND() const noexcept override { return m_windowHandle; }

        [[nodiscard]] Allocator *GetAllocator() const noexcept override { return m_allocator; }

        [[nodiscard]] VkInstance GetVkInstance() const noexcept override { return m_vkInstance; }

        [[nodiscard]] VkPhysicalDevice GetVkPhysicalDevice() const noexcept override { return m_physicalDevice; }

        [[nodiscard]] VkDevice GetVkDevice() const noexcept override { return m_logicalDevice; }

        [[nodiscard]] VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() const noexcept override { return m_debugMessenger; }

        [[nodiscard]] VkSurfaceKHR GetVkSurfaceKHR() const noexcept override { return m_surface; }

        [[nodiscard]] VkQueue GetGraphicsVkQueue() const noexcept override { return m_graphicsQueue; }

        [[nodiscard]] VkQueue GetPresentVkQueue() const noexcept override { return m_presentQueue; }

        [[nodiscard]]  VkCommandPool GetVkCommandPool() const noexcept override { return m_commandPool; }

        [[nodiscard]]  uint32_t GetSwapchainImageCount() const noexcept override { return m_swapchainImageCount; }

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const noexcept override { return m_swapChain; };

        [[nodiscard]] std::vector <VkImage> GetSwapChainImages() const noexcept override { return m_swapChainImages; };

        [[nodiscard]] VkFormat GetSwapChainImageFormat() const noexcept override { return m_swapChainImageFormat; };

        [[nodiscard]] VkExtent2D GetSwapChainExtent() const noexcept override { return m_swapChainExtent; };

        [[nodiscard]] std::vector <VkImageView> GetSwapChainImageViews() const noexcept override { return m_swapChainImageViews; };

    private:
        void CreateInstance();

        void SetupDebugMessenger();

        void CreateSurface();

        void PickPhysicalDevice();

        void CreateLogicalDevice();

        void CreateCommandPool();

        void CreateSwapChain();

        void CreateImageViews();

        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    private:
        const std::vector<const char *> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        const char *validationLayerName = "VK_LAYER_KHRONOS_validation";

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif
        HINSTANCE m_windowInstance;
        HWND m_windowHandle;
        Allocator *m_allocator;
        VkInstance m_vkInstance;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_logicalDevice;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkSurfaceKHR m_surface;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        VkCommandPool m_commandPool;

        uint32_t m_swapchainImageCount = 0;
        VkSwapchainKHR m_swapChain;
        std::vector <VkImage> m_swapChainImages;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector <VkImageView> m_swapChainImageViews;
    };
}

#endif //JOY_GRAPHICS_CONTEXT_H
