#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	class Swapchain
	{
	public:
		Swapchain();

		~Swapchain();

		[[nodiscard]] uint32_t GetSwapchainImageCount() const noexcept { return m_swapchainImageCount; }

		[[nodiscard]] VkSwapchainKHR GetSwapChain() const noexcept { return m_swapChain; };

		[[nodiscard]] std::vector<VkImage> GetSwapChainImages() const noexcept { return m_swapChainImages; };

		[[nodiscard]] VkFormat GetSwapChainImageFormat() const noexcept { return m_swapChainImageFormat; };

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; };
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; };

		[[nodiscard]] std::vector<VkImageView> GetSwapChainImageViews() const noexcept { return m_swapChainImageViews; }
		;

	private:
		uint32_t m_swapchainImageCount = 0;
		uint32_t m_width;
		uint32_t m_height;
		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		std::vector<VkImageView> m_swapChainImageViews;
	};
}

#endif //SWAPCHAIN_H
