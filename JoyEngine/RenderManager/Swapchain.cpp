#include "Swapchain.h"

#include "JoyContext.h"

#include "MemoryManager/MemoryManager.h"

#include "RenderManager/VulkanTypes.h"
#include "RenderManager/VulkanUtils.h"

#include "ResourceManager/ResourceManager.h"

namespace JoyEngine {

    Swapchain::Swapchain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
                JoyContext::Graphics()->GetVkPhysicalDevice(),
                JoyContext::Graphics()->GetVkSurfaceKHR());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, JoyContext::Graphics()->GetHWND());

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && m_swapchainImageCount > swapChainSupport.capabilities.maxImageCount) {
            m_swapchainImageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = JoyContext::Graphics()->GetVkSurfaceKHR();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices queueFamilies = findQueueFamilies(
                JoyContext::Graphics()->GetVkPhysicalDevice(),
                JoyContext::Graphics()->GetVkSurfaceKHR());
        uint32_t queueFamilyIndices[] = {queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value()};

        if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(
                JoyContext::Graphics()->GetVkDevice(),
                &createInfo,
                JoyContext::Graphics()->GetAllocationCallbacks(),
                &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(JoyContext::Graphics()->GetVkDevice(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(JoyContext::Graphics()->GetVkDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapchainImageCount = m_swapChainImages.size();
        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        m_swapChainImageViews.resize(m_swapchainImageCount);

        for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
            MemoryManager::CreateImageView(
                    JoyContext::Graphics()->GetVkDevice(),
                    JoyContext::Graphics()->GetAllocationCallbacks(),
                    m_swapChainImages[i],
                    m_swapChainImageFormat,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    m_swapChainImageViews[i]);
        }
    }

    Swapchain::~Swapchain() {
        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            vkDestroyImageView(JoyContext::Graphics()->GetVkDevice(), m_swapChainImageViews[i], JoyContext::Graphics()->GetAllocationCallbacks());
        }

        vkDestroySwapchainKHR(JoyContext::Graphics()->GetVkDevice(), m_swapChain, JoyContext::Graphics()->GetAllocationCallbacks());
    }
}