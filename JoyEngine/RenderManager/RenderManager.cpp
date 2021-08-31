#include "RenderManager.h"

#include <windows.h>
#include <array>
#include <set>
#include <chrono>

#include "ResourceManager/ResourceManager.h"
#include "MemoryManager/MemoryManager.h"
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

    RenderManager *RenderManager::m_instance = nullptr;

    RenderManager::RenderManager(IJoyGraphicsContext *const graphicsContext,
                                 ResourceManager *const resourceManager) :
            m_graphicsContext(graphicsContext),
            m_resourceManager(resourceManager),
            m_allocator(m_graphicsContext->GetAllocationCallbacks()),
            m_swapchain(new Swapchain(graphicsContext)) {
        m_instance = this;
        CreateRenderPass();
        CreateDepthResources();
        CreateFramebuffers();
    }

    void RenderManager::Stop() {
        vkQueueWaitIdle(m_graphicsContext->GetPresentVkQueue());
        vkDeviceWaitIdle(m_graphicsContext->GetVkDevice());
    }

    RenderManager::~RenderManager() {
        for (auto &item: m_renderObjects) {
            item.second = nullptr;
        }

        m_depthTexture = nullptr;

        for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(m_graphicsContext->GetVkDevice(), m_swapChainFramebuffers[i], m_allocator);
        }

        vkFreeCommandBuffers(m_graphicsContext->GetVkDevice(),
                             m_graphicsContext->GetVkCommandPool(),
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());

        vkDestroyRenderPass(m_graphicsContext->GetVkDevice(), m_renderPass, m_allocator);

        m_swapchain = nullptr;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(m_graphicsContext->GetVkDevice(), m_renderFinishedSemaphores[i], m_allocator);
            vkDestroySemaphore(m_graphicsContext->GetVkDevice(), m_imageAvailableSemaphores[i], m_allocator);
            vkDestroyFence(m_graphicsContext->GetVkDevice(), m_inFlightFences[i], m_allocator);
        }
    }

    void RenderManager::Init() {
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void RenderManager::CreateRenderPass() {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat(m_graphicsContext->GetVkPhysicalDevice());
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
        colorAttachment.format = m_swapchain->GetSwapChainImageFormat();
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

        if (vkCreateRenderPass(m_graphicsContext->GetVkDevice(), &renderPassInfo, m_allocator, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    uint32_t RenderManager::RegisterMeshRenderer(MeshRenderer *meshRenderer) {
        m_renderObjects.insert({
                                       m_renderObjectIndex,
                                       std::make_unique<RenderObject>(meshRenderer, m_graphicsContext, m_renderPass, m_swapchain.get())
                               });
        return ++m_renderObjectIndex;
    }

    void RenderManager::UnregisterMeshRenderer(uint32_t index) {
        if (m_renderObjects.find(index) == m_renderObjects.end()) {
            assert(false);
        }
        m_renderObjects.erase(index);
    }

    void RenderManager::CreateDepthResources() {
        m_depthTexture = std::make_unique<GFXTexture>();
        VkFormat depthFormat = findDepthFormat(m_graphicsContext->GetVkPhysicalDevice());

        MemoryManager::CreateImage(m_graphicsContext->GetVkPhysicalDevice(),
                                   m_graphicsContext->GetVkDevice(),
                                   m_allocator,
                                   m_swapchain->GetSwapChainExtent().width,
                                   m_swapchain->GetSwapChainExtent().height,
                                   depthFormat,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   m_depthTexture->GetImage(),
                                   m_depthTexture->GetDeviceMemory());
        MemoryManager::CreateImageView(m_graphicsContext->GetVkDevice(),
                                       m_allocator,
                                       m_depthTexture->GetImage(),
                                       depthFormat,
                                       VK_IMAGE_ASPECT_DEPTH_BIT,
                                       m_depthTexture->GetImageView());
    }

    void RenderManager::CreateFramebuffers() {
        m_swapChainFramebuffers.resize(m_swapchain->GetSwapchainImageCount());
        for (size_t i = 0; i < m_swapchain->GetSwapchainImageCount(); i++) {
            std::array<VkImageView, 2> attachments = {
                    m_swapchain->GetSwapChainImageViews()[i],
                    m_depthTexture->GetImageView()
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapchain->GetSwapChainExtent().width;
            framebufferInfo.height = m_swapchain->GetSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_graphicsContext->GetVkDevice(),
                                    &framebufferInfo,
                                    m_allocator,
                                    &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void RenderManager::CreateCommandBuffers() {

        commandBuffers.resize(m_swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_graphicsContext->GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(m_graphicsContext->GetVkDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void RenderManager::WriteCommandBuffers(uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain->GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        for (auto const &x: m_renderObjects) {
            RenderObject *ro = x.second.get();
            VkBuffer vertexBuffers[] = {
                    m_resourceManager->GetMesh(ro->GetMeshRenderer()->GetMesh()->GetGuid())->GetVertexBuffer()
            };
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[imageIndex],
                                 m_resourceManager->GetMesh(ro->GetMeshRenderer()->GetMesh()->GetGuid())->GetIndexBuffer(),
                                 0,
                                 VK_INDEX_TYPE_UINT32);

            vkCmdBindPipeline(commandBuffers[imageIndex],
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              ro->GetPipeline());

            vkCmdBindDescriptorSets(commandBuffers[imageIndex],
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    ro->GetPipelineLayout(), 0, 1,
                                    &ro->GetDescriptorSet()[imageIndex], 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[imageIndex],
                             static_cast<uint32_t>(m_resourceManager->GetMesh(ro->GetMeshRenderer()->GetMesh()->GetGuid())->GetIndexSize()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void RenderManager::ResetCommandBuffers(uint32_t imageIndex) {
        vkResetCommandBuffer(commandBuffers[imageIndex], 0);
    }

    void RenderManager::CreateSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imagesInFlight.resize(m_swapchain->GetSwapchainImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_graphicsContext->GetVkDevice(), &semaphoreInfo, m_allocator, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_graphicsContext->GetVkDevice(), &semaphoreInfo, m_allocator, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_graphicsContext->GetVkDevice(), &fenceInfo, m_allocator, &m_inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void RenderManager::Update() {
        DrawFrame();
    }

    void RenderManager::DrawFrame() {
        auto count = m_swapchain->GetSwapchainImageCount();
        vkWaitForFences(m_graphicsContext->GetVkDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_graphicsContext->GetVkDevice(),
                                                m_swapchain->GetSwapChain(),
                                                UINT64_MAX,
                                                m_imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE,
                                                &imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        for (auto const &x: m_renderObjects) {
            x.second->UpdateUniformBuffer(imageIndex);
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_graphicsContext->GetVkDevice(),
                            1,
                            &m_imagesInFlight[imageIndex],
                            VK_TRUE,
                            UINT64_MAX);
            ResetCommandBuffers(imageIndex);
        }
        WriteCommandBuffers(imageIndex);

        // Mark the image as now being in use by this frame
        m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];
        vkResetFences(m_graphicsContext->GetVkDevice(), 1, &m_inFlightFences[currentFrame]);

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};

        VkSubmitInfo submitInfo{
                VK_STRUCTURE_TYPE_SUBMIT_INFO,
                nullptr,
                1,
                waitSemaphores,
                waitStages,
                1,
                &commandBuffers[imageIndex],
                1,
                signalSemaphores
        };

        if (vkQueueSubmit(m_graphicsContext->GetGraphicsVkQueue(), 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkSwapchainKHR swapChains[] = {m_swapchain->GetSwapChain()};
        VkPresentInfoKHR presentInfo{
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                nullptr,
                1,
                signalSemaphores,
                1,
                swapChains,
                &imageIndex,
                nullptr
        };
        result = vkQueuePresentKHR(m_graphicsContext->GetPresentVkQueue(), &presentInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}
