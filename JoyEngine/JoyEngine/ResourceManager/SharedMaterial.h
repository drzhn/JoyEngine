#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include <map>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Common/Resource.h"
#include "Shader.h"
#include "Utils/GUID.h"
#include "Common/Serialization.h"

namespace JoyEngine {

    struct BindingInfo {
        uint32_t bindingIndex;
        std::string type;
        uint32_t count;
        size_t offset;
    };

    class SharedMaterial final : public Resource {
    public :
        SharedMaterial() = delete;

        explicit SharedMaterial(GUID);

        ~SharedMaterial() final;

        [[nodiscard]] VkPipeline GetPipeline() const noexcept;

        [[nodiscard]] VkPipelineLayout GetPipelineLayout() const noexcept;

        BindingInfo GetBindingInfoByName(const std::string &name) noexcept;

        [[nodiscard]] uint64_t GetSetLayoutHash() const noexcept;

        static VkDescriptorType GetTypeFromStr(const std::string &type) noexcept;

    	[[nodiscard]] bool IsLoaded() const noexcept override;
    private :
        GUID m_shaderGuid;
        bool m_hasVertexInput = false;
        bool m_hasMVP = false;
        bool m_depthTest = false;
        bool m_depthWrite = false;

        VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
        uint64_t m_setLayoutHash;
        std::map<std::string, BindingInfo> m_bindings;
        Shader* m_shader = nullptr;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    private:

        GUID m_guid;

    private:
        void Initialize();

        void CreateGraphicsPipeline();
    
    };
}

#endif //SHARED_MATERIAL_H
