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

        [[nodiscard]] const VkAllocationCallbacks *GetAllocationCallbacks() const noexcept override {
//            return nullptr;
            return m_allocator->GetAllocationCallbacks();
        }

        [[nodiscard]] VkInstance GetVkInstance() const noexcept override { return m_vkInstance; }

        [[nodiscard]] VkPhysicalDevice GetVkPhysicalDevice() const noexcept override { return m_physicalDevice; }

        [[nodiscard]] VkDevice GetVkDevice() const noexcept override { return m_logicalDevice; }

        [[nodiscard]] VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() const noexcept override { return m_debugMessenger; }

        [[nodiscard]] VkSurfaceKHR GetVkSurfaceKHR() const noexcept override { return m_surface; }

        [[nodiscard]] VkQueue GetGraphicsVkQueue() const noexcept override { return m_graphicsQueue; }

        [[nodiscard]] VkQueue GetPresentVkQueue() const noexcept override { return m_presentQueue; }

        [[nodiscard]]  VkCommandPool GetVkCommandPool() const noexcept override { return m_commandPool; }

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

#endif //JOY_GRAPHICS_CONTEXT_H
