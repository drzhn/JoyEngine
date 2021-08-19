#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vulkan/vulkan.h>

#include "RenderManager/RenderManager.h"
#include "IJoyGraphicsContext.h"

#include "Components/MeshRenderer.h"
#include "Components/MeshRendererTypes.h"
#include "Swapchain.h"

namespace JoyEngine {
    class RenderObject {
    public :
        RenderObject() = delete;

        explicit RenderObject(MeshRenderer *meshRenderer,
                              IJoyGraphicsContext *graphicsContext,
                              VkRenderPass renderPass,
                              Swapchain *swapchain);

        ~RenderObject();

        [[nodiscard]] const MeshRenderer *GetMeshRenderer() const noexcept { return m_meshRenderer; }

        [[nodiscard]] VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }

        [[nodiscard]] VkPipeline GetPipeline() const { return m_graphicsPipeline; }

        [[nodiscard]] std::vector<VkDescriptorSet> GetDescriptorSet() const { return m_descriptorSets; }

        void UpdateUniformBuffer(uint32_t currentImage);

    private:
        IJoyGraphicsContext *const m_graphicsContext;
        const VkAllocationCallbacks *m_allocator;
        MeshRenderer *const m_meshRenderer;
        Swapchain *m_swapchain;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;

        void CreateDescriptorSetLayout();

        void CreateGraphicsPipeline();

        void CreateUniformBuffers();

        void CreateDescriptorPool();

        void CreateDescriptorSets();

    };
}


#endif //RENDER_OBJECT_H
