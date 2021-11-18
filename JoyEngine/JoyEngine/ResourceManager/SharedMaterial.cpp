#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>

#include "JoyContext.h"
#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	SharedMaterial::SharedMaterial(GUID guid) : Resource(guid), m_guid(guid)
	{
		Initialize();
		CreateGraphicsPipeline();
	}

	void SharedMaterial::Initialize()
	{
		rapidjson::Document json = JoyContext::Data->GetSerializedData(m_guid, sharedMaterial);

		m_shaderGuid = GUID::StringToGuid(json["shader"].GetString());

		m_shader = JoyContext::Resource->LoadResource<Shader>(m_shaderGuid);

		m_hasVertexInput = json["hasVertexInput"].GetBool();
		m_hasMVP = json["hasMVP"].GetBool();
		m_depthTest = json["depthTest"].GetBool();
		m_depthWrite = json["depthWrite"].GetBool();

		const auto bindingsArray = json["bindings"].GetArray();
		const uint32_t bindingsArraySize = bindingsArray.Size();

		std::vector<VkDescriptorSetLayoutBinding> bindings(bindingsArraySize);
		std::vector<VkDescriptorType> types(bindingsArraySize);
		std::vector<size_t> bindingCurrentOffsets(bindingsArraySize, 0);

		int32_t maxBindingIndex = -1;
		for (uint32_t i = 0; i < bindingsArraySize; i++)
		{
			std::string typeStr = bindingsArray[i]["type"].GetString();
			std::string nameStr = bindingsArray[i]["name"].GetString();
			int32_t bindingIndex = bindingsArray[i]["index"].GetInt();
			uint32_t count = bindingsArray[i]["count"].GetUint();

			if (bindingIndex > maxBindingIndex)
			{
				maxBindingIndex = bindingIndex;
			}

			m_bindings.insert({
				nameStr, {
					static_cast<uint32_t>(bindingIndex),
					typeStr,
					count,
					bindingCurrentOffsets[bindingIndex]
				}
			});
			const VkDescriptorType type = GetTypeFromStr(typeStr);
			if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				bindingCurrentOffsets[bindingIndex] += SerializationUtils::GetTypeSize(typeStr);
			}

			types[bindingIndex] = type;
			const VkShaderStageFlags stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
			const uint32_t descriptorCount = 1;
			bindings[bindingIndex] = {
				static_cast<uint32_t>(bindingIndex),
				type,
				descriptorCount,
				stageFlagBits,
				nullptr
			};
		}

		uint64_t hash = 0;
		if (maxBindingIndex != -1)
		{
			for (uint32_t i = 0; i <= maxBindingIndex; i++)
			{
				uint64_t binding_hash = i
					| bindings[i].descriptorType << 8
					| bindings[i].descriptorCount << 16
					| bindings[i].stageFlags << 24;
				hash ^= binding_hash;
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(maxBindingIndex),
				bindings.data()
			};

			VkResult res = vkCreateDescriptorSetLayout(
				JoyContext::Graphics->GetDevice(),
				&layoutInfo,
				JoyContext::Graphics->GetAllocationCallbacks(),
				&m_setLayout);
			ASSERT(res == VK_SUCCESS);
			JoyContext::DescriptorSet->RegisterPool(hash, m_setLayout, types);
			m_setLayoutHash = hash;
		}


		//ASSERT(m_setLayouts.size() == m_setLayoutInfos.size());
	}

	void SharedMaterial::CreateGraphicsPipeline()
	{
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = m_shader->GetVertexShadeModule();
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = m_shader->GetFragmentShadeModule();
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

		VkExtent2D swapchainExtent = {
			JoyContext::Render->GetSwapchain()->GetWidth(),
			JoyContext::Render->GetSwapchain()->GetHeight()
		};


		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchainExtent.width;
		viewport.height = (float)swapchainExtent.height;
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
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
			1,
			&m_setLayout,
			1,
			&pushConstantRange
		};

		VkResult res = vkCreatePipelineLayout(JoyContext::Graphics->GetDevice(),
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

		res = vkCreateGraphicsPipelines(JoyContext::Graphics->GetDevice(),
		                                VK_NULL_HANDLE,
		                                1,
		                                &pipelineInfo,
		                                JoyContext::Graphics->GetAllocationCallbacks(),
		                                &m_graphicsPipeline);
		ASSERT(res == VK_SUCCESS);
	}

	SharedMaterial::~SharedMaterial()
	{
		vkDestroyPipeline(JoyContext::Graphics->GetDevice(),
		                  m_graphicsPipeline,
		                  JoyContext::Graphics->GetAllocationCallbacks());
		vkDestroyPipelineLayout(JoyContext::Graphics->GetDevice(),
		                        m_pipelineLayout,
		                        JoyContext::Graphics->GetAllocationCallbacks());

		JoyContext::Resource->UnloadResource(m_shaderGuid);

		vkDestroyDescriptorSetLayout(JoyContext::Graphics->GetDevice(),
		                             m_setLayout,
		                             JoyContext::Graphics->GetAllocationCallbacks());
		JoyContext::DescriptorSet->UnregisterPool(m_setLayoutHash);
	}

	//Shader *SharedMaterial::GetVertexShader() const noexcept {
	//    return JoyContext::Resource->GetResource<Shader>(m_shaderGuid)->GetVertexShadeModule();
	//}

	//Shader *SharedMaterial::GetFragmentShader() const noexcept {
	//    return JoyContext::Resource->GetResource<Shader>(m_fragmentShader);
	//}

	VkPipeline SharedMaterial::GetPipeline() const noexcept
	{
		return m_graphicsPipeline;
	}

	VkPipelineLayout SharedMaterial::GetPipelineLayout() const noexcept
	{
		return m_pipelineLayout;
	}

	VkDescriptorType SharedMaterial::GetTypeFromStr(const std::string& type) noexcept
	{
		switch (strHash(type.c_str()))
		{
		case strHash("texture"):
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		case strHash("int"):
		case strHash("uint"):
		case strHash("float"):
		case strHash("vec2"):
		case strHash("vec3"):
		case strHash("vec4"):
		case strHash("mat3"):
		case strHash("mat4"):
		case strHash("color"):
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		default:
			ASSERT(false);
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	bool SharedMaterial::IsLoaded() const noexcept
	{
		return m_shader->IsLoaded();
	}

	BindingInfo SharedMaterial::GetBindingInfoByName(const std::string& name) noexcept
	{
		return m_bindings[name];
	}

	uint64_t SharedMaterial::GetSetLayoutHash() const noexcept
	{
		return m_setLayoutHash;
	}
}
