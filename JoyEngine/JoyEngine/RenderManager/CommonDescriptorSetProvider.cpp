#include "CommonDescriptorSetProvider.h"


#include "JoyContext.h"
#include "RenderManager.h"
#include "Common/Time.h"
#include "GraphicsManager/GraphicsManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "Utils/Assert.h"


namespace JoyEngine
{
	CommonDescriptorSetProvider::CommonDescriptorSetProvider()
	{
		uint32_t defines[] = {strHash(JoyVariablesStr), strHash(GBufferTexturesStr)};

		for (const auto& defineHash : defines)
		{
			m_data[defineHash] = std::make_unique<SharedBindingData>();
			SharedBindingData* data = m_data[defineHash].get();

			uint32_t bindingsCount = 0;
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			std::vector<VkDescriptorType> types;
			VkShaderStageFlags stageFlagBits = 0;
			data->setLayoutHash = 0;

			switch (defineHash)
			{
			case strHash(JoyVariablesStr):
				{
					bindingsCount = 1;
					bindings.resize(bindingsCount);
					types = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};
					stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
					data->setIndex = 1;

					std::vector<std::unique_ptr<Buffer>> buffers;

					for (uint32_t i = 0; i < JoyContext::Render->GetSwapchain()->GetSwapchainImageCount(); i++)
					{
						buffers.emplace_back(std::make_unique<Buffer>(
							sizeof(JoyData),
							VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
						));
					}

					data->m_bindings.emplace_back(BindingBase{
						VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						std::move(buffers),
						nullptr,
						{}
					});
					break;
				}
			case strHash(GBufferTexturesStr):
				{
					bindingsCount = 2;
					bindings.resize(bindingsCount);
					types = {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};
					stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
					data->setIndex = 2;

					data->m_bindings.emplace_back(BindingBase
						{
							VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
							{},
							JoyContext::Render->GetGBufferPositionTexture(),
							{}
						});
					data->m_bindings.emplace_back(BindingBase
						{
							VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
							{},
							JoyContext::Render->GetGBufferNormalTexture(),
							{}
						}
					);
					break;
				}
			default:
				{
					ASSERT(false);
				}
			}

			for (uint32_t i = 0; i < bindingsCount; i++)
			{
				bindings[i] = {
					i,
					types[i],
					1,
					stageFlagBits,
					nullptr
				};
				uint64_t binding_hash = i
					| bindings[i].descriptorType << 8
					| bindings[i].descriptorCount << 16
					| bindings[i].stageFlags << 24;
				data->setLayoutHash ^= binding_hash;
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				nullptr,
				0,
				bindingsCount,
				bindings.data()
			};

			VkResult res = vkCreateDescriptorSetLayout(
				JoyContext::Graphics->GetDevice(),
				&layoutInfo,
				JoyContext::Graphics->GetAllocationCallbacks(),
				&data->setLayout);
			ASSERT(res == VK_SUCCESS);
			JoyContext::DescriptorSet->RegisterPool(data->setLayoutHash, data->setLayout, types);
		}

		CreateDescriptorSets();
	}


	void CommonDescriptorSetProvider::SetCamera(Camera* camera)
	{
		m_camera = camera;
	}

	void CommonDescriptorSetProvider::CreateDescriptorSets()
	{
		for (const auto& pair : m_data)
		{
			const uint32_t defineHash = pair.first;
			SharedBindingData* data = m_data[defineHash].get();

			uint32_t imageCount = JoyContext::Render->GetSwapchain()->GetSwapchainImageCount();

			data->descriptorSets = JoyContext::DescriptorSet->Allocate(
				data->setLayoutHash,
				imageCount);

			std::vector<VkWriteDescriptorSet> descriptorWrites(imageCount);

			for (int i = 0; i < data->m_bindings.size(); i++)
			{
				VkDescriptorImageInfo* imageInfoPtr = nullptr;
				VkDescriptorBufferInfo* bufferInfoPtr = nullptr;
				VkBufferView* texelBufferViewPtr = nullptr;

				VkDescriptorImageInfo imageInfo = {};
				VkDescriptorBufferInfo bufferInfo = {};
				VkBufferView texelBufferView = {};

				for (uint32_t j = 0; j < imageCount; j++)
				{
					switch (defineHash)
					{
					case strHash(JoyVariablesStr):
						{
							bufferInfo = {
								data->m_bindings[i].buffers[j]->GetBuffer(),
								0,
								sizeof(JoyData),

							};
							bufferInfoPtr = &bufferInfo;
							break;
						}
					case strHash(GBufferTexturesStr):
						{
							imageInfo = {
								data->m_bindings[i].texture->GetSampler(),
								data->m_bindings[i].texture->GetImageView(),
								VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
							};
							imageInfoPtr = &imageInfo;
							break;
						}
					default:
						{
							ASSERT(false);
						}
					}

					descriptorWrites[j] =
					{
						VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						nullptr,
						data->descriptorSets[j],
						static_cast<uint32_t>(i),
						0,
						1,
						data->m_bindings[i].type,
						imageInfoPtr,
						bufferInfoPtr,
						texelBufferViewPtr
					};
				}
				vkUpdateDescriptorSets(
					JoyContext::Graphics->GetDevice(),
					static_cast<uint32_t>(descriptorWrites.size()),
					descriptorWrites.data(),
					0,
					nullptr);
			}
		}
	}

	void CommonDescriptorSetProvider::UpdateDescriptorSetData(uint32_t imageIndex) const
	{
		std::unique_ptr<BufferMappedPtr> ptr =
			m_data.find(strHash(JoyVariablesStr))->second->
			m_bindings[0].buffers[imageIndex]->
			GetMappedPtr(0, sizeof(JoyData));

		JoyData data{
			m_camera->GetTransform()->GetPosition(),
			m_camera->GetProjMatrix(),
			Time::GetTime(),
			Time::GetDeltaTime()
		};
		memcpy(ptr->GetMappedPtr(), &data, sizeof(data));
	}

	SharedBindingData* CommonDescriptorSetProvider::GetBindingData(uint32_t defineHash)
	{
		ASSERT(m_data.find(defineHash) != m_data.end());
		return m_data.find(defineHash)->second.get();
	}

	CommonDescriptorSetProvider::~CommonDescriptorSetProvider()
	{
		for (const auto& pair : m_data)
		{
			JoyContext::DescriptorSet->Free(pair.second->descriptorSets);
		}
	}

	SharedBindingData::~SharedBindingData()
	{
		vkDestroyDescriptorSetLayout(
			JoyContext::Graphics->GetDevice(),
			setLayout,
			JoyContext::Graphics->GetAllocationCallbacks());
		if (setLayoutHash != 0)
		{
			JoyContext::DescriptorSet->UnregisterPool(setLayoutHash);
		}
	}
}
