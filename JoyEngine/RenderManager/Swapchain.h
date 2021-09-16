#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "JoyGraphicsContext.h"

namespace JoyEngine {
    class Swapchain {
    public:
        Swapchain();

        ~Swapchain();

        [[nodiscard]] uint32_t GetSwapchainImageCount() const noexcept { return m_swapchainImageCount; }

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const noexcept { return m_swapChain; };

        [[nodiscard]] std::vector<VkImage> GetSwapChainImages() const noexcept { return m_swapChainImages; };

        [[nodiscard]] VkFormat GetSwapChainImageFormat() const noexcept { return m_swapChainImageFormat; };

        [[nodiscard]] VkExtent2D GetSwapChainExtent() const noexcept { return m_swapChainExtent; };

        [[nodiscard]] std::vector<VkImageView> GetSwapChainImageViews() const noexcept { return m_swapChainImageViews; };

    private:
        uint32_t m_swapchainImageCount = 0;
        VkSwapchainKHR m_swapChain;
        std::vector<VkImage> m_swapChainImages;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector<VkImageView> m_swapChainImageViews;
    };
}

#endif //SWAPCHAIN_H
