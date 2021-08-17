#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vulkan/vulkan.h>

#include "RenderManager/RenderManager.h"

#include "Components/MeshRenderer.h"
#include "Components/MeshRendererTypes.h"

namespace JoyEngine {
    class MeshRenderer;

    class IJoyGraphicsContext;

    class RenderObject {
    public :
        RenderObject() = delete;

        explicit RenderObject(MeshRenderer *meshRenderer,
                              const IJoyGraphicsContext &m_graphicsContext,
                              VkRenderPass renderPass);

        ~RenderObject();

    private:
        const IJoyGraphicsContext &m_graphicsContext;
        MeshRenderer *m_meshRenderer;
        VkRenderPass m_renderPass;
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_graphicsPipeline;

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
