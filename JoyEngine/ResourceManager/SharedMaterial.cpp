#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>

#include "JoyContext.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine {
    SharedMaterial::SharedMaterial(GUID guid) :Resource(guid) {
        m_guid = guid;
        Initialize();
        CreateGraphicsPipeline();
    }

    void SharedMaterial::Initialize() {
        rapidjson::Document json = JoyContext::Data->GetSerializedData(m_guid, sharedMaterial);

        m_vertexShader = GUID::StringToGuid(json["vertexShader"].GetString());
        m_fragmentShader = GUID::StringToGuid(json["fragmentShader"].GetString());

        JoyContext::Resource->LoadResource<Shader>(m_vertexShader);
        JoyContext::Resource->LoadResource<Shader>(m_fragmentShader);

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
                types[bindingIndex] = type;
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
            VkResult res = vkCreateDescriptorSetLayout(JoyContext::Graphics->GetVkDevice(),
                                                       &layoutInfo,
                                                       JoyContext::Graphics->GetAllocationCallbacks(),
                                                       &setLayout);
            ASSERT(res == VK_SUCCESS);

            m_setLayouts.push_back(setLayout);
            m_setLayoutInfos.push_back({isStatic, hash});
            JoyContext::DescriptorSet->RegisterPool(hash, setLayout, types);
            setIndex++;
        }

        ASSERT(m_setLayouts.size() == m_setLayoutInfos.size());
    }

    void SharedMaterial::CreateGraphicsPipeline() {
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = GetVertexShader()->GetShadeModule();
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = GetFragmentShader()->GetShadeModule();
        fragShaderStageInfo.pName = "main";

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkExtent2D swapchainExtent = JoyContext::Render->GetSwapchain()->GetSwapChainExtent();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapchainExtent.width;
        viewport.height = (float) swapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPushConstantRange pushConstantRange{
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(MVP)
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                nullptr,
                0,
                static_cast<uint32_t>(m_setLayouts.size()),
                m_setLayouts.data(),
                1,
                &pushConstantRange
        };

        VkResult res = vkCreatePipelineLayout(JoyContext::Graphics->GetVkDevice(),
                                              &pipelineLayoutInfo,
                                              JoyContext::Graphics->GetAllocationCallbacks(),
                                              &m_pipelineLayout);
        ASSERT(res == VK_SUCCESS);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = JoyContext::Render->GetMainRenderPass();
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        res = vkCreateGraphicsPipelines(JoyContext::Graphics->GetVkDevice(),
                                        VK_NULL_HANDLE,
                                        1,
                                        &pipelineInfo,
                                        JoyContext::Graphics->GetAllocationCallbacks(),
                                        &m_graphicsPipeline);
        ASSERT(res == VK_SUCCESS);
    }

    SharedMaterial::~SharedMaterial() {
        vkDestroyPipeline(JoyContext::Graphics->GetVkDevice(),
                          m_graphicsPipeline,
                          JoyContext::Graphics->GetAllocationCallbacks());
        vkDestroyPipelineLayout(JoyContext::Graphics->GetVkDevice(),
                                m_pipelineLayout,
                                JoyContext::Graphics->GetAllocationCallbacks());

        JoyContext::Resource->UnloadResource(m_vertexShader);
        JoyContext::Resource->UnloadResource(m_fragmentShader);

        for (uint32_t i = 0; i < m_setLayouts.size(); i++) {
            vkDestroyDescriptorSetLayout(JoyContext::Graphics->GetVkDevice(),
                                         m_setLayouts[i],
                                         JoyContext::Graphics->GetAllocationCallbacks());
            JoyContext::DescriptorSet->UnregisterPool(m_setLayoutInfos[i].hash);
        }
    }

    Shader *SharedMaterial::GetVertexShader() const noexcept {
        return JoyContext::Resource->GetResource<Shader>(m_vertexShader);
    }

    Shader *SharedMaterial::GetFragmentShader() const noexcept {
        return JoyContext::Resource->GetResource<Shader>(m_fragmentShader);
    }

    VkPipeline SharedMaterial::GetPipeline() const noexcept {
        return m_graphicsPipeline;
    }

    VkPipelineLayout SharedMaterial::GetPipelineLayout() const noexcept {
        return m_pipelineLayout;
    }

    VkDescriptorType SharedMaterial::GetTypeFromStr(const std::string &type) noexcept {
        if (type == "texture")
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        if (type == "buffer")
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ASSERT(false);
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    BindingInfo SharedMaterial::GetBindingInfoByName(const std::string &name) noexcept {
        return m_bindings[name];
    }

    SetLayoutInfo SharedMaterial::GetSetLayoutInfo(uint32_t setIndex) noexcept {
        return m_setLayoutInfos[setIndex];
    }

    uint32_t SharedMaterial::GetSetLayoutSize() const noexcept {
        return m_setLayouts.size();
    }
}