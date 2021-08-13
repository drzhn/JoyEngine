#include "RenderManager.h"

#include <windows.h>
#include <array>
#include <set>
#include <chrono>


//#include "RenderManager/GpuAllocator.h"
#include "RenderManager/VulkanAllocator.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/VulkanUtils.h"

#include <Libs/glm/glm/glm.hpp>
#include <Libs/stb/stb_image.h>
#include <Libs/tinyobjloader/tiny_obj_loader.h>
#include <array>


#define GLM_FORCE_RADIANS
#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define TINYOBJLOADER_IMPLEMENTATION

namespace JoyEngine {

    RenderManager::RenderManager(const IJoyGraphicsContext &graphicsContext) :
            m_graphicsContext(graphicsContext) {
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
    }

    void RenderManager::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
                m_graphicsContext.GetVkPhysicalDevice(), m_graphicsContext.GetVkSurfaceKHR());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_graphicsContext.GetHWND());

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_graphicsContext.GetVkSurfaceKHR();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices queueFamilies = findQueueFamilies(m_graphicsContext.GetVkPhysicalDevice(), m_graphicsContext.GetVkSurfaceKHR());
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

        if (vkCreateSwapchainKHR(m_graphicsContext.GetVkDevice(), &createInfo, m_graphicsContext.GetAllocator()->GetAllocationCallbacks(), &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_graphicsContext.GetVkDevice(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_graphicsContext.GetVkDevice(), swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void RenderManager::CreateImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    VkImageView RenderManager::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_graphicsContext.GetVkDevice(), &viewInfo, m_graphicsContext.GetAllocator()->GetAllocationCallbacks(), &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void RenderManager::CreateRenderPass() {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat(m_graphicsContext.GetVkPhysicalDevice());
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_graphicsContext.GetVkDevice(), &renderPassInfo, m_graphicsContext.GetAllocator()->GetAllocationCallbacks(), &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
//
//    class HelloTriangleApplication {
//
//    public:
//
//        void initWindow(HINSTANCE _instance, HWND _windowHandle) {
//            winAppInstance = _instance;
//            windowHandle = _windowHandle;
//        }
//
//        void initVulkan() {
//            createInstance();
//            setupDebugMessenger();
//            createSurface();
//            pickPhysicalDevice();
//            createLogicalDevice();
//            createSwapChain();
//            createImageViews();
//            createRenderPass();
//            createDescriptorSetLayout();
//            createGraphicsPipeline();
//            createCommandPool();
//            createDepthResources();
//            createFramebuffers();
//            createTextureImage();
//            createTextureImageView();
//            createTextureSampler();
//            loadModel();
//            createVertexBuffer();
//            createIndexBuffer();
//            createUniformBuffers();
//            createDescriptorPool();
//            createDescriptorSets();
//            createCommandBuffers();
//            createSyncObjects();
//        }
//
////        void mainLoop() {
////            while (!wantToExit.load(std::memory_order_acquire)) {
////                drawFrame();
////            }
////
////            vkDeviceWaitIdle(logicalDevice);
////            cleanup();
////        }
////
////        void stopLoop() {
////            wantToExit.store(true, std::memory_order_release);
////        }
////
////        void waitForDeviceIdle() {
////            vkDeviceWaitIdle(logicalDevice);
////        }
//
//        void drawFrame() {
//            vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
//
//            uint32_t imageIndex;
//            VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
//
//            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//                framebufferResized = false;
//                recreateSwapChain();
//                return;
//            } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//                throw std::runtime_error("failed to acquire swap chain image!");
//            }
//
//
//            // Check if a previous frame is using this image (i.e. there is its fence to wait on)
//            if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
//                vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
//            }
//            // Mark the image as now being in use by this frame
//            imagesInFlight[imageIndex] = inFlightFences[currentFrame];
//
//            updateUniformBuffer(imageIndex);
//
//            VkSubmitInfo submitInfo{};
//            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//            VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
//
//            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//            submitInfo.waitSemaphoreCount = 1;
//            submitInfo.pWaitSemaphores = waitSemaphores;
//            submitInfo.pWaitDstStageMask = waitStages;
//            submitInfo.commandBufferCount = 1;
//            submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
//
//            VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
//            submitInfo.signalSemaphoreCount = 1;
//            submitInfo.pSignalSemaphores = signalSemaphores;
//
//            vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
//
//            if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
//                throw std::runtime_error("failed to submit draw command buffer!");
//            }
//
//            VkPresentInfoKHR presentInfo{};
//            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//            presentInfo.waitSemaphoreCount = 1;
//            presentInfo.pWaitSemaphores = signalSemaphores;
//
//            VkSwapchainKHR swapChains[] = {swapChain};
//            presentInfo.swapchainCount = 1;
//            presentInfo.pSwapchains = swapChains;
//            presentInfo.pImageIndices = &imageIndex;
//            presentInfo.pResults = nullptr; // Optional
//
//            result = vkQueuePresentKHR(presentQueue, &presentInfo);
//
//            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
//                framebufferResized = false;
//                recreateSwapChain();
//            } else if (result != VK_SUCCESS) {
//                throw std::runtime_error("failed to present swap chain image!");
//            }
//
//            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//
//            vkQueueWaitIdle(presentQueue);
//            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//        }
//
//        void cleanup() {
//            cleanupSwapChain();
//
//
//            vkDestroySampler(logicalDevice, textureSampler, allocator_->GetAllocationCallbacks());
//            vkDestroyImageView(logicalDevice, textureImageView, allocator_->GetAllocationCallbacks());
//            vkDestroyImage(logicalDevice, textureImage, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, textureImageMemory, allocator_->GetAllocationCallbacks());
//
//            vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, allocator_->GetAllocationCallbacks());
//
//            vkDestroyBuffer(logicalDevice, indexBuffer, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, indexBufferMemory, allocator_->GetAllocationCallbacks());
//
//            vkDestroyBuffer(logicalDevice, vertexBuffer, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, vertexBufferMemory, allocator_->GetAllocationCallbacks());
//
//            for (size_t i = 0; i < swapChainImages.size(); i++) {
//                vkDestroyBuffer(logicalDevice, uniformBuffers[i], allocator_->GetAllocationCallbacks());
//                vkFreeMemory(logicalDevice, uniformBuffersMemory[i], allocator_->GetAllocationCallbacks());
//            }
//
//            vkDestroyDescriptorPool(logicalDevice, descriptorPool, allocator_->GetAllocationCallbacks());
//
//            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//                vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], allocator_->GetAllocationCallbacks());
//                vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], allocator_->GetAllocationCallbacks());
//                vkDestroyFence(logicalDevice, inFlightFences[i], allocator_->GetAllocationCallbacks());
//            }
//
//            vkDestroyCommandPool(logicalDevice, commandPool, allocator_->GetAllocationCallbacks());
//            vkDestroyDevice(logicalDevice, allocator_->GetAllocationCallbacks());
//
//            if (enableValidationLayers) {
//                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocator_->GetAllocationCallbacks());
//            }
//
//            vkDestroySurfaceKHR(instance, surface, allocator_->GetAllocationCallbacks());
//            vkDestroyInstance(instance, allocator_->GetAllocationCallbacks());
//
//        }
//
//    private:
//
//        HINSTANCE winAppInstance;
//        HWND windowHandle;
////        std::atomic<bool> wantToExit = false;
//
//
//        VkInstance instance;
//        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
//        VkDebugUtilsMessengerEXT debugMessenger;
//        VkSurfaceKHR surface;
//
//        VkDevice logicalDevice;
//        VkQueue graphicsQueue;
//        VkQueue presentQueue;
//
//        VkSwapchainKHR swapChain;
//        std::vector <VkImage> swapChainImages;
//        VkFormat swapChainImageFormat;
//        VkExtent2D swapChainExtent;
//        std::vector <VkImageView> swapChainImageViews;
//
//        VkRenderPass renderPass;
//        VkDescriptorSetLayout descriptorSetLayout;
//        VkPipelineLayout pipelineLayout;
//        VkPipeline graphicsPipeline;
//
//        std::vector <VkFramebuffer> swapChainFramebuffers;
//        VkCommandPool commandPool;
//        std::vector <VkCommandBuffer> commandBuffers;
//
//        std::vector <VkSemaphore> imageAvailableSemaphores;
//        std::vector <VkSemaphore> renderFinishedSemaphores;
//        std::vector <VkFence> inFlightFences;
//        std::vector <VkFence> imagesInFlight;
//        size_t currentFrame = 0;
//
//        bool framebufferResized = false;
//
//        std::vector <Vertex> vertices;
//        std::vector <uint32_t> indices;
//
//        VkBuffer vertexBuffer;
//        VkDeviceMemory vertexBufferMemory;
//
//        VkBuffer indexBuffer;
//        VkDeviceMemory indexBufferMemory;
//
//        std::vector <VkBuffer> uniformBuffers;
//        std::vector <VkDeviceMemory> uniformBuffersMemory;
//
//        VkDescriptorPool descriptorPool;
//        std::vector <VkDescriptorSet> descriptorSets;
//
//        VkImage textureImage;
//        VkDeviceMemory textureImageMemory;
//        VkImageView textureImageView;
//        VkSampler textureSampler;
//
//        VkImage depthImage;
//        VkDeviceMemory depthImageMemory;
//        VkImageView depthImageView;
//
//        Allocator *allocator_;
//
//
//        //    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
//        //        auto app = reinterpret_cast<HelloTriangleApplication *>(glfwGetWindowUserPointer(window));
//        //        app->framebufferResized = true;
//        //    }
//
//
//        void createDepthResources() {
//            VkFormat depthFormat = findDepthFormat();
//
//            createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                        depthImage,
//                        depthImageMemory);
//            depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
//        }
//
//        VkFormat findSupportedFormat(const std::vector <VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
//            for (VkFormat format : candidates) {
//                VkFormatProperties props;
//                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
//
//                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
//                    return format;
//                } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
//                    return format;
//                }
//            }
//
//            throw std::runtime_error("failed to find supported format!");
//        }
//
//        VkFormat findDepthFormat() {
//            return findSupportedFormat(
//                    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
//                    VK_IMAGE_TILING_OPTIMAL,
//                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
//                    );
//        }
//
//        void createTextureImage() {
//            int texWidth, texHeight, texChannels;
//            stbi_uc *pixels = stbi_load(TEXTURE_PATH, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//            VkDeviceSize imageSize = texWidth * texHeight * 4;
//
//            if (!pixels) {
//                throw std::runtime_error("failed to load texture image!");
//            }
//
//            VkBuffer stagingBuffer;
//            VkDeviceMemory stagingBufferMemory;
//            createBuffer(physicalDevice, logicalDevice, imageSize,
//                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                         stagingBuffer, stagingBufferMemory);
//
//            void *data;
//            vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
//            memcpy(data, pixels, static_cast<size_t>(imageSize));
//            vkUnmapMemory(logicalDevice, stagingBufferMemory);
//
//            stbi_image_free(pixels);
//
//            createImage(texWidth, texHeight,
//                        VK_FORMAT_R8G8B8A8_SRGB,
//                        VK_IMAGE_TILING_OPTIMAL,
//                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                        textureImage, textureImageMemory);
//
//            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
//                                  VK_IMAGE_LAYOUT_UNDEFINED,
//                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//            copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
//                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//
//            vkDestroyBuffer(logicalDevice, stagingBuffer, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, stagingBufferMemory, allocator_->GetAllocationCallbacks());
//        }
//
//        void createTextureImageView() {
//            textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
//        }
//
//        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
//            VkImageViewCreateInfo viewInfo{};
//            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//            viewInfo.image = image;
//            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//            viewInfo.format = format;
//            viewInfo.subresourceRange.aspectMask = aspectFlags;
//            viewInfo.subresourceRange.baseMipLevel = 0;
//            viewInfo.subresourceRange.levelCount = 1;
//            viewInfo.subresourceRange.baseArrayLayer = 0;
//            viewInfo.subresourceRange.layerCount = 1;
//
//            VkImageView imageView;
//            if (vkCreateImageView(logicalDevice, &viewInfo, allocator_->GetAllocationCallbacks(), &imageView) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create texture image view!");
//            }
//
//            return imageView;
//        }
//
//        void
//        createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {
//            VkImageCreateInfo imageInfo{};
//            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//            imageInfo.imageType = VK_IMAGE_TYPE_2D;
//            imageInfo.extent.width = width;
//            imageInfo.extent.height = height;
//            imageInfo.extent.depth = 1;
//            imageInfo.mipLevels = 1;
//            imageInfo.arrayLayers = 1;
//            imageInfo.format = format;
//            imageInfo.tiling = tiling;
//            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//            imageInfo.usage = usage;
//            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//            if (vkCreateImage(logicalDevice, &imageInfo, allocator_->GetAllocationCallbacks(), &image) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create image!");
//            }
//
//            VkMemoryRequirements memRequirements;
//            vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);
//
//            VkMemoryAllocateInfo allocInfo{};
//            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//            allocInfo.allocationSize = memRequirements.size;
//            allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
//
//            std::cout << "Allocation GPU Memory:";
//            if (vkAllocateMemory(logicalDevice, &allocInfo, allocator_->GetAllocationCallbacks(), &imageMemory) != VK_SUCCESS) {
//                throw std::runtime_error("failed to allocate image memory!");
//            }
//            std::cout << "GPU Allocated:";
//
//            vkBindImageMemory(logicalDevice, image, imageMemory, 0);
//        }
//
//        void createTextureSampler() {
//            VkSamplerCreateInfo samplerInfo{};
//            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//            samplerInfo.magFilter = VK_FILTER_LINEAR;
//            samplerInfo.minFilter = VK_FILTER_LINEAR;
//            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//            samplerInfo.anisotropyEnable = VK_TRUE;
//            samplerInfo.maxAnisotropy = 1.0f;
//            VkPhysicalDeviceProperties properties{};
//            vkGetPhysicalDeviceProperties(physicalDevice, &properties);
//            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
//            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//            samplerInfo.unnormalizedCoordinates = VK_FALSE;
//            samplerInfo.compareEnable = VK_FALSE;
//            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//            samplerInfo.mipLodBias = 0.0f;
//            samplerInfo.minLod = 0.0f;
//            samplerInfo.maxLod = 0.0f;
//
//            if (vkCreateSampler(logicalDevice, &samplerInfo, allocator_->GetAllocationCallbacks(), &textureSampler) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create texture sampler!");
//            }
//        }
//
//        VkCommandBuffer beginSingleTimeCommands() {
//            VkCommandBufferAllocateInfo allocInfo{};
//            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//            allocInfo.commandPool = commandPool;
//            allocInfo.commandBufferCount = 1;
//
//            VkCommandBuffer commandBuffer;
//            vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
//
//            VkCommandBufferBeginInfo beginInfo{};
//            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//            vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//            return commandBuffer;
//        }
//
//        void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
//            vkEndCommandBuffer(commandBuffer);
//
//            VkSubmitInfo submitInfo{};
//            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//            submitInfo.commandBufferCount = 1;
//            submitInfo.pCommandBuffers = &commandBuffer;
//
//            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//            vkQueueWaitIdle(graphicsQueue);
//
//            vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
//        }
//
//        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
//            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//
//            VkBufferCopy copyRegion{};
//            copyRegion.size = size;
//            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
//
//            endSingleTimeCommands(commandBuffer);
//        }
//
//        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
//            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//
//            VkImageMemoryBarrier barrier{};
//            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//            barrier.oldLayout = oldLayout;
//            barrier.newLayout = newLayout;
//            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            barrier.image = image;
//            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//            barrier.subresourceRange.baseMipLevel = 0;
//            barrier.subresourceRange.levelCount = 1;
//            barrier.subresourceRange.baseArrayLayer = 0;
//            barrier.subresourceRange.layerCount = 1;
//
//            VkPipelineStageFlags sourceStage;
//            VkPipelineStageFlags destinationStage;
//
//            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//                barrier.srcAccessMask = 0;
//                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//
//                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//
//                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//            } else {
//                throw std::invalid_argument("unsupported layout transition!");
//            }
//
//            vkCmdPipelineBarrier(
//                    commandBuffer,
//                    sourceStage, destinationStage,
//                    0,
//                    0, nullptr,
//                    0, nullptr,
//                    1, &barrier
//                    );
//
//            endSingleTimeCommands(commandBuffer);
//        }
//
//        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
//            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//
//            VkBufferImageCopy region{};
//            region.bufferOffset = 0;
//            region.bufferRowLength = 0;
//            region.bufferImageHeight = 0;
//            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//            region.imageSubresource.mipLevel = 0;
//            region.imageSubresource.baseArrayLayer = 0;
//            region.imageSubresource.layerCount = 1;
//            region.imageOffset = {0, 0, 0};
//            region.imageExtent = {
//                    width,
//                    height,
//                    1
//            };
//
//            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
//
//            endSingleTimeCommands(commandBuffer);
//        }
//
//        void createBuffer(VkPhysicalDevice pDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
//            VkBufferCreateInfo bufferInfo{};
//            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//            bufferInfo.size = size;
//            bufferInfo.usage = usage;
//            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//            if (vkCreateBuffer(device, &bufferInfo, allocator_->GetAllocationCallbacks(), &buffer) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create buffer!");
//            }
//
//            VkMemoryRequirements memRequirements;
//            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
//
//            VkMemoryAllocateInfo allocInfo{};
//            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//            allocInfo.allocationSize = memRequirements.size;
//            allocInfo.memoryTypeIndex = findMemoryType(pDevice, memRequirements.memoryTypeBits, properties);
//
//            if (vkAllocateMemory(device, &allocInfo, allocator_->GetAllocationCallbacks(), &bufferMemory) != VK_SUCCESS) {
//                throw std::runtime_error("failed to allocate buffer memory!");
//            }
//
//            vkBindBufferMemory(device, buffer, bufferMemory, 0);
//        }
//
//        void loadModel() {
//            tinyobj::attrib_t attrib;
//            std::vector <tinyobj::shape_t> shapes;
//            std::vector <tinyobj::material_t> materials;
//            std::string warn, err;
//
//            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH)) {
//                throw std::runtime_error(warn + err);
//            }
//
//            for (const auto &shape : shapes) {
//                for (const auto &index : shape.mesh.indices) {
//                    Vertex vertex{};
//
//                    vertex.pos = {
//                            attrib.vertices[3 * index.vertex_index + 0],
//                            attrib.vertices[3 * index.vertex_index + 1],
//                            attrib.vertices[3 * index.vertex_index + 2]
//                    };
//
//                    vertex.texCoord = {
//                            attrib.texcoords[2 * index.texcoord_index + 0],
//                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//                    };
//
//                    vertex.color = {1.0f, 1.0f, 1.0f};
//
//                    vertices.push_back(vertex);
//                    indices.push_back(indices.size());
//                }
//            }
//        }
//
//        void createVertexBuffer() {
//            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
//
//            VkBuffer stagingBuffer;
//            VkDeviceMemory stagingBufferMemory;
//            createBuffer(physicalDevice, logicalDevice, bufferSize,
//                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                         stagingBuffer,
//                         stagingBufferMemory);
//
//            void *data;
//            vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
//            memcpy(data, vertices.data(), (size_t) bufferSize);
//            vkUnmapMemory(logicalDevice, stagingBufferMemory);
//
//            createBuffer(physicalDevice, logicalDevice, bufferSize,
//                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                         vertexBuffer,
//                         vertexBufferMemory);
//
//            copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
//
//            vkDestroyBuffer(logicalDevice, stagingBuffer, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, stagingBufferMemory, allocator_->GetAllocationCallbacks());
//        }
//
//        static uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//            VkPhysicalDeviceMemoryProperties memProperties;
//            vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
//
//            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//                    return i;
//                }
//            }
//
//            throw std::runtime_error("failed to find suitable memory type!");
//        }
//
//        void createIndexBuffer() {
//            VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
//
//            VkBuffer stagingBuffer;
//            VkDeviceMemory stagingBufferMemory;
//            createBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
//                         stagingBufferMemory);
//
//            void *data;
//            vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
//            memcpy(data, indices.data(), (size_t) bufferSize);
//            vkUnmapMemory(logicalDevice, stagingBufferMemory);
//
//            createBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,
//                         indexBufferMemory);
//
//            copyBuffer(stagingBuffer, indexBuffer, bufferSize);
//
//            vkDestroyBuffer(logicalDevice, stagingBuffer, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, stagingBufferMemory, allocator_->GetAllocationCallbacks());
//        }
//
//        void createDescriptorSetLayout() {
//            VkDescriptorSetLayoutBinding uboLayoutBinding{};
//            uboLayoutBinding.binding = 0;
//            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//            uboLayoutBinding.descriptorCount = 1; // Number of objects in array. For single uniform object this value will be 1
//            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//            uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
//
//            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
//            samplerLayoutBinding.binding = 1;
//            samplerLayoutBinding.descriptorCount = 1;
//            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//            samplerLayoutBinding.pImmutableSamplers = nullptr;
//            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//
//            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
//            VkDescriptorSetLayoutCreateInfo layoutInfo{};
//            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//            layoutInfo.pBindings = bindings.data();
//
//            if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, allocator_->GetAllocationCallbacks(), &descriptorSetLayout) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create descriptor set layout!");
//            }
//        }
//
//        void createUniformBuffers() {
//            VkDeviceSize bufferSize = sizeof(UniformBufferObject);
//
//            for (size_t i = 0; i < uniformBuffers.size(); i++) {
//                vkDestroyBuffer(logicalDevice, uniformBuffers[i], allocator_->GetAllocationCallbacks());
//                vkFreeMemory(logicalDevice, uniformBuffersMemory[i], allocator_->GetAllocationCallbacks());
//            }
//
//            uniformBuffers.resize(swapChainImages.size());
//            uniformBuffersMemory.resize(swapChainImages.size());
//
//            for (size_t i = 0; i < swapChainImages.size(); i++) {
//                createBuffer(physicalDevice, logicalDevice,
//                             bufferSize,
//                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                             uniformBuffers[i], uniformBuffersMemory[i]);
//            }
//        }
//
//        void createDescriptorPool() {
//            if (descriptorPool != VK_NULL_HANDLE) {
//                vkDestroyDescriptorPool(logicalDevice, descriptorPool, allocator_->GetAllocationCallbacks());
//            }
//
//            std::array<VkDescriptorPoolSize, 2> poolSizes{};
//            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//            poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
//            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//            poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
//
//            VkDescriptorPoolCreateInfo poolInfo{};
//            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//            poolInfo.pPoolSizes = poolSizes.data();
//            poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
//
//            if (vkCreateDescriptorPool(logicalDevice, &poolInfo, allocator_->GetAllocationCallbacks(), &descriptorPool) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create descriptor pool!");
//            }
//        }
//
//        void createDescriptorSets() {
//            std::vector <VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
//            VkDescriptorSetAllocateInfo allocInfo{};
//            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//            allocInfo.descriptorPool = descriptorPool;
//            allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
//            allocInfo.pSetLayouts = layouts.data();
//
//            descriptorSets.resize(swapChainImages.size());
//            if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
//                throw std::runtime_error("failed to allocate descriptor sets!");
//            }
//
//            for (size_t i = 0; i < swapChainImages.size(); i++) {
//                VkDescriptorBufferInfo bufferInfo{};
//                bufferInfo.buffer = uniformBuffers[i];
//                bufferInfo.offset = 0;
//                bufferInfo.range = sizeof(UniformBufferObject);
//
//                VkDescriptorImageInfo imageInfo{};
//                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                imageInfo.imageView = textureImageView;
//                imageInfo.sampler = textureSampler;
//
//                std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
//
//                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//                descriptorWrites[0].dstSet = descriptorSets[i];
//                descriptorWrites[0].dstBinding = 0;
//                descriptorWrites[0].dstArrayElement = 0;
//                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//                descriptorWrites[0].descriptorCount = 1;
//                descriptorWrites[0].pBufferInfo = &bufferInfo;
//
//                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//                descriptorWrites[1].dstSet = descriptorSets[i];
//                descriptorWrites[1].dstBinding = 1;
//                descriptorWrites[1].dstArrayElement = 0;
//                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//                descriptorWrites[1].descriptorCount = 1;
//                descriptorWrites[1].pImageInfo = &imageInfo;
//
//                vkUpdateDescriptorSets(logicalDevice,
//                                       static_cast<uint32_t>(descriptorWrites.size()),
//                                       descriptorWrites.data(),
//                                       0,
//                                       nullptr);
//            }
//        }
//
//        void recreateSwapChain() {
//            int width = 0, height = 0;
//            RECT rect;
//            if (GetWindowRect(windowHandle, &rect)) {
//                width = rect.right - rect.left;
//                height = rect.bottom - rect.top;
//            }
//
//
//            vkDeviceWaitIdle(logicalDevice);
//
//            cleanupSwapChain();
//
//            createSwapChain();
//            createImageViews();
//            createRenderPass();
//            createGraphicsPipeline();
//            createDepthResources();
//            createFramebuffers();
//            createUniformBuffers();
//            createDescriptorPool();
//            createDescriptorSets();
//            createCommandBuffers();
//        }
//
//        void cleanupSwapChain() {
//            vkDestroyImageView(logicalDevice, depthImageView, allocator_->GetAllocationCallbacks());
//            vkDestroyImage(logicalDevice, depthImage, allocator_->GetAllocationCallbacks());
//            vkFreeMemory(logicalDevice, depthImageMemory, allocator_->GetAllocationCallbacks());
//
//            for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
//                vkDestroyFramebuffer(logicalDevice, swapChainFramebuffers[i], allocator_->GetAllocationCallbacks());
//            }
//
//            vkFreeCommandBuffers(logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
//
//            vkDestroyPipeline(logicalDevice, graphicsPipeline, allocator_->GetAllocationCallbacks());
//            vkDestroyPipelineLayout(logicalDevice, pipelineLayout, allocator_->GetAllocationCallbacks());
//            vkDestroyRenderPass(logicalDevice, renderPass, allocator_->GetAllocationCallbacks());
//
//            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
//                vkDestroyImageView(logicalDevice, swapChainImageViews[i], allocator_->GetAllocationCallbacks());
//            }
//
//            vkDestroySwapchainKHR(logicalDevice, swapChain, allocator_->GetAllocationCallbacks());
//        }
//
//        void createSyncObjects() {
//            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
//            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
//            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
//            imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
//
//            VkSemaphoreCreateInfo semaphoreInfo{};
//            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//            VkFenceCreateInfo fenceInfo{};
//            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//                if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, allocator_->GetAllocationCallbacks(), &imageAvailableSemaphores[i]) != VK_SUCCESS ||
//                vkCreateSemaphore(logicalDevice, &semaphoreInfo, allocator_->GetAllocationCallbacks(), &renderFinishedSemaphores[i]) != VK_SUCCESS ||
//                vkCreateFence(logicalDevice, &fenceInfo, allocator_->GetAllocationCallbacks(), &inFlightFences[i]) != VK_SUCCESS) {
//
//                    throw std::runtime_error("failed to create synchronization objects for a frame!");
//                }
//            }
//        }
//
//        void createCommandBuffers() {
//            commandBuffers.resize(swapChainFramebuffers.size());
//
//            VkCommandBufferAllocateInfo allocInfo{};
//            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//            allocInfo.commandPool = commandPool;
//            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
//
//            if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
//                throw std::runtime_error("failed to allocate command buffers!");
//            }
//
//            for (size_t i = 0; i < commandBuffers.size(); i++) {
//                VkCommandBufferBeginInfo beginInfo{};
//                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//                if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
//                    throw std::runtime_error("failed to begin recording command buffer!");
//                }
//
//                VkRenderPassBeginInfo renderPassInfo{};
//                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//                renderPassInfo.renderPass = renderPass;
//                renderPassInfo.framebuffer = swapChainFramebuffers[i];
//                renderPassInfo.renderArea.offset = {0, 0};
//                renderPassInfo.renderArea.extent = swapChainExtent;
//
//                std::array<VkClearValue, 2> clearValues{};
//                clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
//                clearValues[1].depthStencil = {1.0f, 0};
//
//                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
//                renderPassInfo.pClearValues = clearValues.data();
//
//                vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//                VkBuffer vertexBuffers[] = {vertexBuffer};
//                VkDeviceSize offsets[] = {0};
//                vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
//
//                vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//
//                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
//
//                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
//
//                vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
//
//                vkCmdEndRenderPass(commandBuffers[i]);
//
//                if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
//                    throw std::runtime_error("failed to record command buffer!");
//                }
//            }
//        }
//
//        void createCommandPool() {
//            QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
//
//            VkCommandPoolCreateInfo poolInfo{};
//            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
//            poolInfo.flags = 0; // Optional
//            if (vkCreateCommandPool(logicalDevice, &poolInfo, allocator_->GetAllocationCallbacks(), &commandPool) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create command pool!");
//            }
//        }
//
//        void createFramebuffers() {
//            swapChainFramebuffers.resize(swapChainImageViews.size());
//            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
//                std::array<VkImageView, 2> attachments = {
//                        swapChainImageViews[i],
//                        depthImageView
//                };
//
//                VkFramebufferCreateInfo framebufferInfo{};
//                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//                framebufferInfo.renderPass = renderPass;
//                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//                framebufferInfo.pAttachments = attachments.data();
//                framebufferInfo.width = swapChainExtent.width;
//                framebufferInfo.height = swapChainExtent.height;
//                framebufferInfo.layers = 1;
//
//                if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, allocator_->GetAllocationCallbacks(), &swapChainFramebuffers[i]) != VK_SUCCESS) {
//                    throw std::runtime_error("failed to create framebuffer!");
//                }
//            }
//        }
//
//        void createRenderPass() {
//            VkAttachmentDescription depthAttachment{};
//            depthAttachment.format = findDepthFormat();
//            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//            VkAttachmentReference depthAttachmentRef{};
//            depthAttachmentRef.attachment = 1;
//            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//            VkAttachmentDescription colorAttachment{};
//            colorAttachment.format = swapChainImageFormat;
//            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//            VkAttachmentReference colorAttachmentRef{};
//            colorAttachmentRef.attachment = 0;
//            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//            VkSubpassDescription subpass{};
//            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//            subpass.colorAttachmentCount = 1;
//            subpass.pColorAttachments = &colorAttachmentRef;
//            subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//            VkSubpassDependency dependency{};
//            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//            dependency.dstSubpass = 0;
//            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//            dependency.srcAccessMask = 0;
//            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//            std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
//            VkRenderPassCreateInfo renderPassInfo{};
//            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//            renderPassInfo.pAttachments = attachments.data();
//            renderPassInfo.subpassCount = 1;
//            renderPassInfo.pSubpasses = &subpass;
//            renderPassInfo.dependencyCount = 1;
//            renderPassInfo.pDependencies = &dependency;
//
//            if (vkCreateRenderPass(logicalDevice, &renderPassInfo, allocator_->GetAllocationCallbacks(), &renderPass) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create render pass!");
//            }
//        }
//
//        void createGraphicsPipeline() {
//            auto vertShaderCode = readFile("../shaders/vert.spv");
//            auto fragShaderCode = readFile("../shaders/frag.spv");
//
//            VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//            VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
//
//            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//            vertShaderStageInfo.module = vertShaderModule;
//            vertShaderStageInfo.pName = "main";
//
//            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//            fragShaderStageInfo.module = fragShaderModule;
//            fragShaderStageInfo.pName = "main";
//
//            auto bindingDescription = Vertex::getBindingDescription();
//            auto attributeDescriptions = Vertex::getAttributeDescriptions();
//
//            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
//
//            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//            vertexInputInfo.vertexBindingDescriptionCount = 1;
//            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//            inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//            VkViewport viewport{};
//            viewport.x = 0.0f;
//            viewport.y = 0.0f;
//            viewport.width = (float) swapChainExtent.width;
//            viewport.height = (float) swapChainExtent.height;
//            viewport.minDepth = 0.0f;
//            viewport.maxDepth = 1.0f;
//
//            VkRect2D scissor{};
//            scissor.offset = {0, 0};
//            scissor.extent = swapChainExtent;
//
//            VkPipelineViewportStateCreateInfo viewportState{};
//            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//            viewportState.viewportCount = 1;
//            viewportState.pViewports = &viewport;
//            viewportState.scissorCount = 1;
//            viewportState.pScissors = &scissor;
//
//            VkPipelineRasterizationStateCreateInfo rasterizer{};
//            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//            rasterizer.depthClampEnable = VK_FALSE;
//            rasterizer.rasterizerDiscardEnable = VK_FALSE;
//            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//            rasterizer.lineWidth = 1.0f;
//            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//            rasterizer.depthBiasEnable = VK_FALSE;
//            rasterizer.depthBiasConstantFactor = 0.0f; // Optional
//            rasterizer.depthBiasClamp = 0.0f; // Optional
//            rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
//
//            VkPipelineMultisampleStateCreateInfo multisampling{};
//            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//            multisampling.sampleShadingEnable = VK_FALSE;
//            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//            multisampling.minSampleShading = 1.0f; // Optional
//            multisampling.pSampleMask = nullptr; // Optional
//            multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
//            multisampling.alphaToOneEnable = VK_FALSE; // Optional
//
//            VkPipelineDepthStencilStateCreateInfo depthStencil{};
//            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//            depthStencil.depthTestEnable = VK_TRUE;
//            depthStencil.depthWriteEnable = VK_TRUE;
//            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
//            depthStencil.depthBoundsTestEnable = VK_FALSE;
//            depthStencil.stencilTestEnable = VK_FALSE;
//
//            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//            colorBlendAttachment.blendEnable = VK_FALSE;
//            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
//            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
//
//            VkPipelineColorBlendStateCreateInfo colorBlending{};
//            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//            colorBlending.logicOpEnable = VK_FALSE;
//            colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
//            colorBlending.attachmentCount = 1;
//            colorBlending.pAttachments = &colorBlendAttachment;
//            colorBlending.blendConstants[0] = 0.0f; // Optional
//            colorBlending.blendConstants[1] = 0.0f; // Optional
//            colorBlending.blendConstants[2] = 0.0f; // Optional
//            colorBlending.blendConstants[3] = 0.0f; // Optional
//
//            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//            pipelineLayoutInfo.setLayoutCount = 1;
//            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
//
//            if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, allocator_->GetAllocationCallbacks(), &pipelineLayout) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create pipeline layout!");
//            }
//
//            VkGraphicsPipelineCreateInfo pipelineInfo{};
//            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//            pipelineInfo.stageCount = 2;
//            pipelineInfo.pStages = shaderStages;
//            pipelineInfo.pVertexInputState = &vertexInputInfo;
//            pipelineInfo.pInputAssemblyState = &inputAssembly;
//            pipelineInfo.pViewportState = &viewportState;
//            pipelineInfo.pRasterizationState = &rasterizer;
//            pipelineInfo.pMultisampleState = &multisampling;
//            pipelineInfo.pColorBlendState = &colorBlending;
//            pipelineInfo.layout = pipelineLayout;
//            pipelineInfo.renderPass = renderPass;
//            pipelineInfo.pDepthStencilState = &depthStencil;
//            pipelineInfo.subpass = 0;
//            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//
//            if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, allocator_->GetAllocationCallbacks(), &graphicsPipeline) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create graphics pipeline!");
//            }
//
//            vkDestroyShaderModule(logicalDevice, fragShaderModule, allocator_->GetAllocationCallbacks());
//            vkDestroyShaderModule(logicalDevice, vertShaderModule, allocator_->GetAllocationCallbacks());
//        }
//
//        VkShaderModule createShaderModule(const std::vector<char> &code) {
//            VkShaderModuleCreateInfo createInfo{};
//            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//            createInfo.codeSize = code.size();
//            createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
//            VkShaderModule shaderModule;
//            if (vkCreateShaderModule(logicalDevice, &createInfo, allocator_->GetAllocationCallbacks(), &shaderModule) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create shader module!");
//            }
//            return shaderModule;
//        }
//
//        void createImageViews() {
//            swapChainImageViews.resize(swapChainImages.size());
//
//            for (uint32_t i = 0; i < swapChainImages.size(); i++) {
//                swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
//            }
//        }
//
//        void createSwapChain() {
//            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
//
//            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
//            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
//            VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
//
//            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
//            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//                imageCount = swapChainSupport.capabilities.maxImageCount;
//            }
//
//            VkSwapchainCreateInfoKHR createInfo{};
//            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//            createInfo.surface = surface;
//
//            createInfo.minImageCount = imageCount;
//            createInfo.imageFormat = surfaceFormat.format;
//            createInfo.imageColorSpace = surfaceFormat.colorSpace;
//            createInfo.imageExtent = extent;
//            createInfo.imageArrayLayers = 1;
//            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//
//            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//            uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
//
//            if (indices.graphicsFamily != indices.presentFamily) {
//                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//                createInfo.queueFamilyIndexCount = 2;
//                createInfo.pQueueFamilyIndices = queueFamilyIndices;
//            } else {
//                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//                createInfo.queueFamilyIndexCount = 0; // Optional
//                createInfo.pQueueFamilyIndices = nullptr; // Optional
//            }
//
//            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
//            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//            createInfo.presentMode = presentMode;
//            createInfo.clipped = VK_TRUE;
//
//            createInfo.oldSwapchain = VK_NULL_HANDLE;
//
//            if (vkCreateSwapchainKHR(logicalDevice, &createInfo, allocator_->GetAllocationCallbacks(), &swapChain) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create swap chain!");
//            }
//
//            vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
//            swapChainImages.resize(imageCount);
//            vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());
//
//            swapChainImageFormat = surfaceFormat.format;
//            swapChainExtent = extent;
//        }
//
//        void createSurface() {
//
//            VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKhr = {
//                    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
//                    nullptr,
//                    0,
//                    winAppInstance,
//                    windowHandle,
//                    };
//            VkResult res = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfoKhr, allocator_->GetAllocationCallbacks(), &surface);
//            if (res != VK_SUCCESS) {
//                throw std::runtime_error("failed to create window surface!");
//            }
//
//        }
//
//        void createLogicalDevice() {
//            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//
//            std::vector <VkDeviceQueueCreateInfo> queueCreateInfos;
//            std::set <uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
//
//            float queuePriority = 1.0f;
//            for (uint32_t queueFamily : uniqueQueueFamilies) {
//                VkDeviceQueueCreateInfo queueCreateInfo{};
//                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//                queueCreateInfo.queueFamilyIndex = queueFamily;
//                queueCreateInfo.queueCount = 1;
//                queueCreateInfo.pQueuePriorities = &queuePriority;
//                queueCreateInfos.push_back(queueCreateInfo);
//            }
//
//            VkPhysicalDeviceFeatures deviceFeatures{};
//            deviceFeatures.samplerAnisotropy = VK_TRUE;
//
//            VkDeviceCreateInfo createInfo{};
//            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//
//            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//            createInfo.pQueueCreateInfos = queueCreateInfos.data();
//
//            createInfo.pEnabledFeatures = &deviceFeatures;
//
//            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//            createInfo.ppEnabledExtensionNames = deviceExtensions.data();
//
//            if (enableValidationLayers) {
//                createInfo.enabledLayerCount = 1;
//                createInfo.ppEnabledLayerNames = &validationLayerName;
//            } else {
//                createInfo.enabledLayerCount = 0;
//            }
//
//            if (vkCreateDevice(physicalDevice, &createInfo, allocator_->GetAllocationCallbacks(), &logicalDevice) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create logical device!");
//            }
//
//            vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
//            vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
//        }
//
//        void pickPhysicalDevice() {
//            uint32_t deviceCount = 0;
//            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
//            if (deviceCount == 0) {
//                throw std::runtime_error("failed to find GPUs with Vulkan support!");
//            }
//            std::vector <VkPhysicalDevice> devices(deviceCount);
//            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
//            for (const auto &device : devices) {
//                if (isPhysicalDeviceSuitable(device)) {
//                    physicalDevice = device;
//                    break;
//                }
//            }
//            if (physicalDevice == VK_NULL_HANDLE) {
//                throw std::runtime_error("failed to find a suitable GPU!");
//            }
//        }
//
//        bool isPhysicalDeviceSuitable(VkPhysicalDevice device) {
//            QueueFamilyIndices indices = findQueueFamilies(device);
//
//            bool extensionsSupported = checkDeviceExtensionSupport(device);
//
//            bool swapChainAdequate = false;
//            if (extensionsSupported) {
//                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
//                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//            }
//
//            VkPhysicalDeviceFeatures supportedFeatures;
//            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
//
//            return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
//        }
//
//        static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
//            uint32_t extensionCount;
//            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
//
//            std::vector <VkExtensionProperties> availableExtensions(extensionCount);
//            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
//
//            std::set <std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
//
//            for (const auto &extension : availableExtensions) {
//                requiredExtensions.erase(extension.extensionName);
//            }
//
//            return requiredExtensions.empty();
//        }
//
//        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector <VkSurfaceFormatKHR> &availableFormats) {
//            for (const auto &availableFormat : availableFormats) {
//                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//                    return availableFormat;
//                }
//            }
//
//            return availableFormats[0];
//        }
//
//        VkPresentModeKHR chooseSwapPresentMode(const std::vector <VkPresentModeKHR> &availablePresentModes) {
//            for (const auto &availablePresentMode : availablePresentModes) {
//                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//                    return availablePresentMode;
//                }
//            }
//
//            return VK_PRESENT_MODE_FIFO_KHR;
//        }
//
//        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
//            if (capabilities.currentExtent.width != UINT32_MAX) {
//                return capabilities.currentExtent;
//            } else {
//                int width, height;
//                RECT rect;
//                if (GetWindowRect(windowHandle, &rect)) {
//                    width = rect.right - rect.left;
//                    height = rect.bottom - rect.top;
//                }
//
//                VkExtent2D actualExtent = {
//                        static_cast<uint32_t>(width),
//                        static_cast<uint32_t>(height)
//                };
//
//                actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
//                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
//
//                return actualExtent;
//            }
//        }
//
//        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
//            SwapChainSupportDetails details;
//
//            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
//
//            uint32_t formatCount;
//            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
//
//            if (formatCount != 0) {
//                details.formats.resize(formatCount);
//                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
//            }
//
//            uint32_t presentModeCount;
//            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
//
//            if (presentModeCount != 0) {
//                details.presentModes.resize(presentModeCount);
//                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
//            }
//
//            return details;
//        }
//
//        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
//            QueueFamilyIndices indices;
//
//            uint32_t queueFamilyCount = 0;
//            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//
//            std::vector <VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
//
//            int i = 0;
//            for (const auto &queueFamily : queueFamilies) {
//                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//                    indices.graphicsFamily = i;
//                }
//                VkBool32 presentSupport = false;
//                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
//
//                if (presentSupport) {
//                    indices.presentFamily = i;
//                }
//
//                if (indices.isComplete()) {
//                    break;
//                }
//
//                i++;
//            }
//
//            return indices;
//        }
//
//        void updateUniformBuffer(uint32_t currentImage) {
//            static auto startTime = std::chrono::high_resolution_clock::now();
//
//            auto currentTime = std::chrono::high_resolution_clock::now();
//            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
//
//            UniformBufferObject ubo{};
//            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//            ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
//            ubo.proj[1][1] *= -1;
//
//            void *data;
//            vkMapMemory(logicalDevice, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
//            memcpy(data, &ubo, sizeof(ubo));
//            vkUnmapMemory(logicalDevice, uniformBuffersMemory[currentImage]);
//        }
//
//        void createInstance() {
//
//            allocator_ = new Allocator();
//            if (enableValidationLayers && !checkValidationLayerSupport()) {
//                throw std::runtime_error("validation layers requested, but not available!");
//            }
//
//            VkApplicationInfo appInfo{};
//            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//            appInfo.pApplicationName = "Joy Instance";
//            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//            appInfo.pEngineName = "Joy Engine";
//            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//            appInfo.apiVersion = VK_API_VERSION_1_0;
//
//            VkInstanceCreateInfo createInfo{};
//            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//            createInfo.pApplicationInfo = &appInfo;
//
//            auto extensions = getRequiredExtensions();
//            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//            createInfo.ppEnabledExtensionNames = extensions.data();
//
//            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
//            if (enableValidationLayers) {
//                createInfo.enabledLayerCount = 1;
//                createInfo.ppEnabledLayerNames = &validationLayerName;
//
//                populateDebugMessengerCreateInfo(debugCreateInfo);
//                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT * ) & debugCreateInfo;
//            } else {
//                createInfo.enabledLayerCount = 0;
//
//                createInfo.pNext = allocator_->GetAllocationCallbacks();
//            }
//
//            if (vkCreateInstance(&createInfo, allocator_->GetAllocationCallbacks(), &instance) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create instance!");
//            }
//        }
//
//        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
//            createInfo = {};
//            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//            createInfo.messageSeverity =
//                    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
//                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
//                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//            createInfo.messageType =
//                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
//                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
//                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//            createInfo.pfnUserCallback = debugCallback;
//        }
//
//        void setupDebugMessenger() {
//            if (!enableValidationLayers) return;
//
//            VkDebugUtilsMessengerCreateInfoEXT createInfo;
//            populateDebugMessengerCreateInfo(createInfo);
//
//            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, allocator_->GetAllocationCallbacks(), &debugMessenger) != VK_SUCCESS) {
//                throw std::runtime_error("failed to set up debug messenger!");
//            }
//        }
//

//

//
//        static VKAPI_ATTR VkBool32
//
//        VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
//                                 void *pUserData) {
//            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
//
//            return VK_FALSE;
//        }
//    };
}
