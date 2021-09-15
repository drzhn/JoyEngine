#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include <map>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Common/Resource.h"
#include "Shader.h"
#include "Utils/GUID.h"

namespace JoyEngine {

    struct BindingInfo {
        uint32_t setIndex;
        uint32_t bindingIndex;
        VkDescriptorType type;
    };

    struct SetLayoutInfo {
        bool isStatic;
        uint64_t hash;
    };

    class SharedMaterial final : public Resource {
    public :
        SharedMaterial() = delete;

        explicit SharedMaterial(GUID);

        ~SharedMaterial() final;

        [[nodiscard]] Shader *GetVertexShader() const noexcept;

        [[nodiscard]] Shader *GetFragmentShader() const noexcept;

    private :
        GUID m_vertexShader;
        GUID m_fragmentShader;
        bool m_hasVertexInput = false;
        bool m_hasMVP = false;
        bool m_depthTest = false;
        bool m_depthWrite = false;

        std::vector<SetLayoutInfo> m_setLayoutInfos;
        std::vector<VkDescriptorSetLayout> m_setLayouts;
        std::map<std::string, BindingInfo> m_bindings;

//        std::vector<VkDescriptorSet> m_descriptorSets;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    private:
        static VkDescriptorType GetTypeFromStr(const std::string &type) noexcept;

        GUID m_guid;

    private:
        void CreateGraphicsPipeline();
    };
}

#endif //SHARED_MATERIAL_H
