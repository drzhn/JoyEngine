#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>

#include "IJoyGraphicsContext.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/DescriptorSetManager.h"

namespace JoyEngine {
    SharedMaterial::SharedMaterial(GUID guid) {
        m_guid = guid;
        rapidjson::Document json = DataManager::GetInstance()->GetSerializedData(m_guid, sharedMaterial);

        m_vertexShader = GUID::StringToGuid(json["vertexShader"].GetString());
        m_fragmentShader = GUID::StringToGuid(json["fragmentShader"].GetString());

        m_hasVertexInput = json["hasVertexInput"].GetBool();
        m_hasMVP = json["hasMVP"].GetBool();
        m_depthTest = json["depthTest"].GetBool();
        m_depthWrite = json["depthWrite"].GetBool();

        uint32_t setIndex = 0;
        for (const auto &bindingSet: json["bindingSets"].GetArray()) {
            bool isStatic = bindingSet["isStatic"].GetBool();
            uint64_t hash = 0;

            const auto bindingSetArray = bindingSet["bindings"].GetArray();
            const uint32_t bindingSetArraySize = bindingSetArray.Size();
            std::vector<VkDescriptorSetLayoutBinding> bindings(bindingSetArraySize);
            std::vector<VkDescriptorType> types(bindingSetArraySize);
            for (uint32_t bindingIndex = 0; bindingIndex < bindingSetArraySize; bindingIndex++) {
                std::string typeStr = bindingSetArray[bindingIndex]["type"].GetString();
                std::string nameStr = bindingSetArray[bindingIndex]["name"].GetString();
                VkDescriptorType type = GetTypeFromStr(typeStr);
                VkShaderStageFlags stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
                uint32_t descriptorCount = 1;
                bindings[bindingIndex] = {
                        bindingIndex,
                        type,
                        descriptorCount,
                        stageFlagBits, // TODO make it more clean
                        nullptr
                };
                m_bindings.insert({nameStr, {setIndex, bindingIndex, type}});
                uint64_t binding_hash = bindingIndex | type << 8 | descriptorCount << 16 | stageFlagBits << 24;
                hash ^= binding_hash;
            }
            VkDescriptorSetLayoutCreateInfo layoutInfo{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    nullptr,
                    0,
                    bindingSetArraySize,
                    bindings.data()
            };

            VkDescriptorSetLayout setLayout;
            VkResult res = vkCreateDescriptorSetLayout(IJoyGraphicsContext::GetInstance()->GetVkDevice(),
                                                       &layoutInfo,
                                                       IJoyGraphicsContext::GetInstance()->GetAllocationCallbacks(),
                                                       &setLayout);
            ASSERT(res == VK_SUCCESS);

            m_setLayouts.push_back({setLayout, isStatic, hash});
            DescriptorSetManager::GetInstance()->RegisterPool(hash, isStatic, types);
            setIndex++;
        }
    }

    SharedMaterial::~SharedMaterial() {
        ResourceManager::GetInstance()->UnloadResource(m_vertexShader);
        ResourceManager::GetInstance()->UnloadResource(m_fragmentShader);
    }

    Shader *SharedMaterial::GetVertexShader() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Shader>(m_vertexShader);
    }

    Shader *SharedMaterial::GetFragmentShader() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Shader>(m_fragmentShader);
    }

    VkDescriptorType SharedMaterial::GetTypeFromStr(const std::string &type) noexcept {
        if (type == "texture")
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        if (type == "buffer")
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ASSERT(false);
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }


}