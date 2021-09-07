#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Resource.h"
#include "Shader.h"
#include "Utils/GUID.h"

namespace JoyEngine {

    class SharedMaterial final: public Resource {
    public :
        SharedMaterial() = delete;

        explicit SharedMaterial(GUID);

        ~SharedMaterial() final;

        [[nodiscard]] Shader *GetVertexShader() const noexcept;

        [[nodiscard]] Shader *GetFragmentShader() const noexcept;

    private :
        GUID m_vertexShader;
        GUID m_fragmentShader;
        bool m_hasVertexInput;
        bool m_hasMVP;
        bool m_depthTest;
        bool m_depthWrite;
        std::vector<std::vector<std::tuple<std::string, std::string>>> m_bindingSets;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    };
}

#endif //SHARED_MATERIAL_H
