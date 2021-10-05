#include "Material.h"

#include "JoyContext.h"

#include <rapidjson/document.h>

#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine {
    Material::Material(GUID guid) :Resource(guid) {

        rapidjson::Document json = JoyContext::Data->GetSerializedData(guid, material);

        m_sharedMaterialGuid = GUID::StringToGuid(json["sharedMaterial"].GetString());
        JoyContext::Resource->LoadResource<SharedMaterial>(m_sharedMaterialGuid);
        m_sharedMaterial = JoyContext::Resource->GetResource<SharedMaterial>(m_sharedMaterialGuid);

        for (auto &binding: json["bindings"].GetArray()) {
            m_bindings.insert({
                                      binding["name"].GetString(),
                                      GUID::StringToGuid(binding["data"].GetString())
                                      // TODO What about another types of bindings? int, float, color, array, etc...
                              });
        }


        LoadResources();
        CreateDescriptorSets();
    }

    Material::~Material() {
        for (const auto &item: m_descriptorSets) {
            JoyContext::DescriptorSet->Free(item.second);
        }
        JoyContext::Resource->UnloadResource(m_sharedMaterialGuid);
        for (const auto &item: m_bindings) {
            JoyContext::Resource->UnloadResource(item.second);
        }
    }

    void Material::LoadResources() {
        for (const auto &binding: m_bindings) {
            VkDescriptorType type = m_sharedMaterial->GetBindingInfoByName(binding.first).type;
            switch (type) {
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    JoyContext::Resource->LoadResource<Texture>(binding.second);
                    break;
                default:
                    ASSERT(false);
            }
        }
    }

    void Material::CreateDescriptorSets() {
        for (uint32_t i = 0; i < m_sharedMaterial->GetSetLayoutSize(); i++) {
            SetLayoutInfo setLayoutInfo = m_sharedMaterial->GetSetLayoutInfo(i);
            std::vector<VkDescriptorSet> set = JoyContext::DescriptorSet->Allocate(
                    setLayoutInfo.hash,
                    setLayoutInfo.isStatic ? 1 : JoyContext::Render->GetSwapchain()->GetSwapchainImageCount());
            m_descriptorSets.insert({i, set});
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        for (const auto &binding: m_bindings) {

            std::string bindingName = binding.first;
            GUID resourceGuid = binding.second;

            BindingInfo bindingInfo = m_sharedMaterial->GetBindingInfoByName(bindingName);
            SetLayoutInfo setLayoutInfo = m_sharedMaterial->GetSetLayoutInfo(bindingInfo.setIndex);
            VkDescriptorImageInfo *imageInfoPtr = nullptr;
            VkDescriptorBufferInfo *bufferInfoPtr = nullptr;
            VkBufferView *texelBufferViewPtr = nullptr;

            VkDescriptorImageInfo imageInfo = {};
            VkDescriptorBufferInfo bufferInfo = {};
            VkBufferView texelBufferView = {};

            switch (bindingInfo.type) {
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                    Texture *texture = JoyContext::Resource->GetResource<Texture>(resourceGuid);
                    imageInfo = {
                            texture->GetSampler(),
                            texture->GetImageView(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };
                    imageInfoPtr = &imageInfo;
                    break;
                }
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    ASSERT(false);
                    break;
//                    VkDescriptorBufferInfo bufferInfo{};
//                    bufferInfo.buffer = m_uniformBuffers[i];
//                    bufferInfo.offset = 0;
//                    bufferInfo.range = sizeof(MVP);
                default:
                    ASSERT(false);
            }
            for (const auto &descriptorSet: m_descriptorSets[bindingInfo.setIndex]) {
                descriptorWrites.push_back(
                        {
                                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                descriptorSet,
                                bindingInfo.bindingIndex,
                                0,
                                1,
                                bindingInfo.type,
                                imageInfoPtr,
                                bufferInfoPtr,
                                texelBufferViewPtr
                        });
            }
        }
        vkUpdateDescriptorSets(
                JoyContext::Graphics->GetDevice(),
                static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(),
                0,
                nullptr);
    }

    SharedMaterial *Material::GetSharedMaterial() const noexcept {
        return m_sharedMaterial;
    }

    std::map<uint32_t, std::vector<VkDescriptorSet>> Material::GetDescriptorMap() const noexcept {
        return m_descriptorSets;
    }
}