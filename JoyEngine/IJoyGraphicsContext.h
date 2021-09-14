#ifndef I_JOY_GRAPHICS_CONTEXT_H
#define I_JOY_GRAPHICS_CONTEXT_H

#include "windows.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "Utils/Assert.h"
#include "RenderManager/VulkanAllocator.h"

namespace JoyEngine {
    class IJoyGraphicsContext {
    public :
        static IJoyGraphicsContext *GetInstance() noexcept {
            ASSERT(m_instance != nullptr);
            return m_instance;
        }

        [[nodiscard]] virtual HINSTANCE GetHINSTANCE() const noexcept = 0;

        [[nodiscard]] virtual HWND GetHWND() const noexcept = 0;

        [[nodiscard]] virtual const VkAllocationCallbacks *GetAllocationCallbacks() const noexcept = 0;

        [[nodiscard]] virtual VkInstance GetVkInstance() const noexcept = 0;

        [[nodiscard]] virtual VkPhysicalDevice GetVkPhysicalDevice() const noexcept = 0;

        [[nodiscard]] virtual VkDevice GetVkDevice() const noexcept = 0;

        [[nodiscard]] virtual VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() const noexcept = 0;

        [[nodiscard]] virtual VkSurfaceKHR GetVkSurfaceKHR() const noexcept = 0;

        [[nodiscard]] virtual VkQueue GetGraphicsVkQueue() const noexcept = 0;

        [[nodiscard]] virtual VkQueue GetPresentVkQueue() const noexcept = 0;

        [[nodiscard]] virtual VkCommandPool GetVkCommandPool() const noexcept = 0;

    protected:
        static IJoyGraphicsContext *m_instance;
    };
}
#endif //I_JOY_GRAPHICS_CONTEXT_H
