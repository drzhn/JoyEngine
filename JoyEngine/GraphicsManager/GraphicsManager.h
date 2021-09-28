#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <vector>

#include "windows.h"
#include <vulkan/vulkan.h>
#include "RenderManager/VulkanAllocator.h"

namespace JoyEngine {
    class GraphicsManager {
    public:
        GraphicsManager(HINSTANCE instance, HWND windowHandle);

        ~GraphicsManager();

        [[nodiscard]] HINSTANCE GetHINSTANCE() const noexcept { return m_windowInstance; }

        [[nodiscard]] HWND GetHWND() const noexcept { return m_windowHandle; }

        [[nodiscard]] const VkAllocationCallbacks *GetAllocationCallbacks() const noexcept {
//            return nullptr;
            return m_allocator->GetAllocationCallbacks();
        }

        [[nodiscard]] VkInstance GetVkInstance() const noexcept { return m_vkInstance; }

        [[nodiscard]] VkPhysicalDevice GetVkPhysicalDevice() const noexcept { return m_physicalDevice; }

        [[nodiscard]] VkDevice GetVkDevice() const noexcept { return m_logicalDevice; }

        [[nodiscard]] VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() const noexcept { return m_debugMessenger; }

        [[nodiscard]] VkSurfaceKHR GetVkSurfaceKHR() const noexcept { return m_surface; }

        [[nodiscard]] VkQueue GetGraphicsVkQueue() const noexcept { return m_graphicsQueue; }

        [[nodiscard]] VkQueue GetPresentVkQueue() const noexcept { return m_presentQueue; }

        [[nodiscard]]  VkCommandPool GetVkCommandPool() const noexcept { return m_commandPool; }

    private:
        void CreateInstance();

        void SetupDebugMessenger();

        void CreateSurface();

        void PickPhysicalDevice();

        void CreateLogicalDevice();

        void CreateCommandPool();

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
    };
}

#endif //GRAPHICS_MANAGER_H
