#ifndef I_JOY_GRAPHICS_CONTEXT_H
#define I_JOY_GRAPHICS_CONTEXT_H

#include "windows.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "RenderManager/VulkanAllocator.h"

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

        [[nodiscard]] virtual uint32_t GetSwapchainImageCount() const noexcept = 0;

        [[nodiscard]] virtual VkSwapchainKHR GetSwapChain() const noexcept = 0;

        [[nodiscard]] virtual std::vector<VkImage> GetSwapChainImages() const noexcept = 0;

        [[nodiscard]] virtual VkFormat GetSwapChainImageFormat() const noexcept = 0;

        [[nodiscard]] virtual VkExtent2D GetSwapChainExtent() const noexcept = 0;

        [[nodiscard]] virtual std::vector<VkImageView> GetSwapChainImageViews() const noexcept = 0;
    };
}
#endif //I_JOY_GRAPHICS_CONTEXT_H
