#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>

#include "JoyContext.h"
#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "RenderManager/VulkanTypes.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	SharedMaterial::SharedMaterial(GUID guid) : Resource(guid)
	{
		Initialize();
		CreateGraphicsPipeline();
		JoyContext::Render->RegisterSharedMaterial(this);
	}

	void SharedMaterial::Initialize()
	{
		rapidjson::Document json = JoyContext::Data->GetSerializedData(m_guid, sharedMaterial);

		m_shader = GUID::StringToGuid(json["shader"].GetString());

		m_hasVertexInput = json["hasVertexInput"].GetBool();
		m_hasMVP = json["hasMVP"].GetBool();
		m_depthTest = json["depthTest"].GetBool();
		m_depthWrite = json["depthWrite"].GetBool();

		m_colorAttachmentsCount = json["colorAttachmentsCount"].GetUint();
		m_subpassIndex = json["subpassIndex"].GetUint();

		{
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
					bindingCurrentOffsets[bindingIndex] += SerializationUtils::GetTypeSize(typeStr) * count;
				}

				types[bindingIndex] = type;
				VkShaderStageFlags stageFlagBits = 0;
				if (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
				{
					stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
				}
				else
				{
					stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
				}
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
			int bindingsCount = maxBindingIndex + 1;

			m_vulkanBindings.resize(bindingsCount);
			for (uint32_t i = 0; i < bindingsCount; i++)
			{
				uint64_t binding_hash = i
					| bindings[i].descriptorType << 8
					| bindings[i].descriptorCount << 16
					| bindings[i].stageFlags << 24;
				hash ^= binding_hash;

				m_vulkanBindings[i] = {
					bindings[i].descriptorType,
					bindingCurrentOffsets[i]
				};
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(maxBindingIndex + 1),
				bindings.data()
			};

			VkResult res = vkCreateDescriptorSetLayout(
				JoyContext::Graphics->GetDevice(),
				&layoutInfo,
				JoyContext::Graphics->GetAllocationCallbacks(),
				&m_setLayout);
			ASSERT(res == VK_SUCCESS);
			if (bindingsCount > 0)
			{
				JoyContext::DescriptorSet->RegisterPool(hash, m_setLayout, types);
			}
			m_setLayoutHash = hash;
		}

		if (!json.HasMember("bindingDefines")) return;
		const auto bindingsDefinesArray = json["bindingDefines"].GetArray();
		const uint32_t bindingDefinesArraySize = bindingsDefinesArray.Size();
		for (uint32_t i =0; i < bindingDefinesArraySize; i++)
		{
			//for (auto m = bindingsDefinesArray[i].MemberBegin(); m < bindingsDefinesArray[i].MemberEnd(); m++)
			//{
			//	std::string n = m->name.GetString();
			//}
			std::string define = bindingsDefinesArray[i].GetString();
			m_bindingDefines.push_back(strHash(define.c_str()));
		}
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
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachments[] = {
			{
				VK_FALSE,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			},
			{
				VK_FALSE,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			}
		};


		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = m_colorAttachmentsCount;
		colorBlending.pAttachments = colorBlendAttachments;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPushConstantRange pushConstantRange{
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(MVP)
		};

		std::array<VkDescriptorSetLayout, 4> layouts;
		layouts[0] = m_setLayout;
		uint32_t maxLayoutIndex = 0;
		for (const auto& def : m_bindingDefines)
		{
			SharedBindingData* data = JoyContext::Render->GetBindingDataForDefine(def);
			if (data->setIndex > maxLayoutIndex)
			{
				maxLayoutIndex = data->setIndex;
			}
			layouts[data->setIndex] = data->setLayout;
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			maxLayoutIndex+1,
			layouts.data(),
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
		pipelineInfo.subpass = m_subpassIndex;
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

		vkDestroyDescriptorSetLayout(
			JoyContext::Graphics->GetDevice(),
			m_setLayout,
			JoyContext::Graphics->GetAllocationCallbacks());
		if (m_setLayoutHash != 0)
		{
			JoyContext::DescriptorSet->UnregisterPool(m_setLayoutHash);
		}
		JoyContext::Render->UnregisterSharedMaterial(this);
	}

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
		case strHash("attachment"):
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
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
		default:
			ASSERT(false);
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	bool SharedMaterial::IsLoaded() const noexcept
	{
		return m_shader->IsLoaded();
	}

	void SharedMaterial::RegisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		m_meshRenderers.insert(meshRenderer);
	}

	void SharedMaterial::UnregisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		if (m_meshRenderers.find(meshRenderer) == m_meshRenderers.end())
		{
			ASSERT(false);
		}
		m_meshRenderers.erase(meshRenderer);
	}

	std::set<MeshRenderer*>& SharedMaterial::GetMeshRenderers()
	{
		return m_meshRenderers;
	}

	std::vector<uint32_t>& SharedMaterial::GetBindingDefines()
	{
		return m_bindingDefines;
	}

	BindingInfo* SharedMaterial::GetBindingInfoByName(const std::string& name) noexcept
	{
		if (m_bindings.find(name) == m_bindings.end())
		{
			return nullptr;
		}
		return &(m_bindings.find(name)->second);
	}

	uint64_t SharedMaterial::GetSetLayoutHash() const noexcept
	{
		return m_setLayoutHash;
	}

	std::vector<VulkanBindingDescription>& SharedMaterial::GetVulkanBindings()
	{
		return m_vulkanBindings;
	}
}
