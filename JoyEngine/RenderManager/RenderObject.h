#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vulkan/vulkan.h>

#include "RenderManager/RenderManager.h"
#include "IJoyGraphicsContext.h"

#include "Components/MeshRenderer.h"
#include "Components/MeshRendererTypes.h"

namespace JoyEngine {
    class RenderObject {
    public :
        RenderObject() = delete;

        explicit RenderObject(MeshRenderer *meshRenderer,
                              const IJoyGraphicsContext &m_graphicsContext,
                              VkRenderPass renderPass);

        ~RenderObject();

        [[nodiscard]] const MeshRenderer *GetMeshRenderer() const noexcept { return m_meshRenderer; }

        [[nodiscard]] VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }

        [[nodiscard]] VkPipeline GetPipeline() const { return m_graphicsPipeline; }

        [[nodiscard]] std::vector<VkDescriptorSet> GetDescriptorSet() const { return m_descriptorSets; }

        void UpdateUniformBuffer(uint32_t currentImage);

    private:
        const IJoyGraphicsContext &m_graphicsContext;
        const MeshRenderer *m_meshRenderer;
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
