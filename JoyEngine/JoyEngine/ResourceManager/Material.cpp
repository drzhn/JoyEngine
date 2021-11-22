#include "Material.h"
#include <vector>
#include "JoyContext.h"

#include <rapidjson/document.h>

#include "Common/SerializationUtils.h"
#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	Material::Material(GUID guid) : Resource(guid)
	{
		rapidjson::Document json = JoyContext::Data->GetSerializedData(guid, material);

		m_sharedMaterial = GUID::StringToGuid(json["sharedMaterial"].GetString());
		std::vector<VulkanBindingDescription>& vbd = m_sharedMaterial->GetVulkanBindings();
		m_bindings.resize(vbd.size());
		for (int i = 0; i < vbd.size(); i++)
		{
			m_bindings[i].bindingDescription = vbd[i];
			if (vbd[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				for (int j = 0; j < JoyContext::Render->GetSwapchain()->GetSwapchainImageCount(); j++)
				{
					m_bindings[i].buffers.emplace_back(std::make_unique<Buffer>(
						vbd[i].size,
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
					));
				}
			}
		}

		for (auto& binding : json["bindings"].GetArray())
		{
			std::string nameStr = binding["name"].GetString();
			const rapidjson::Value& data = binding["data"];
			BindingInfo info = m_sharedMaterial->GetBindingInfoByName(nameStr);
			if (info.type == "texture")
			{
				ASSERT(data.IsString());
				std::string dataString = data.GetString();
				GUID textureGuid = GUID();
				if (!dataString.empty())
				{
					textureGuid = GUID::StringToGuid(dataString);
					JoyContext::Resource->LoadResource<Texture>(textureGuid);
				}
				m_bindings[info.bindingIndex].textureGuid = textureGuid;
			}
			else if (info.type == "attachment")
			{
				ASSERT(data.IsString());
				std::string dataString = data.GetString();
				ASSERT(!dataString.empty())
				switch (strHash(dataString.c_str()))
				{
				case strHash("position"):
					m_bindings[info.bindingIndex].inputAttachmentType = Position;
					break;
				case strHash("normal"):
					m_bindings[info.bindingIndex].inputAttachmentType = Normal;
					break;
				default:
					ASSERT(false);
				}
			}
			else
			{
				for (int j = 0; j < JoyContext::Render->GetSwapchain()->GetSwapchainImageCount(); j++)
				{
					std::unique_ptr<BufferMappedPtr> ptr = m_bindings[info.bindingIndex].buffers[j]->
						GetMappedPtr(info.offset, info.count * SerializationUtils::GetTypeSize(info.type));

					SerializationUtils::DeserializeToPtr(
						strHash(info.type.c_str()),
						data, ptr->GetMappedPtr(),
						info.count);
				}
			}
		}


		CreateDescriptorSets();
	}

	Material::~Material()
	{
		JoyContext::DescriptorSet->Free(m_descriptorSets);
		for (const auto& item : m_bindings)
		{
			if (item.bindingDescription.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (!item.textureGuid.IsNull())
				{
					JoyContext::Resource->UnloadResource(item.textureGuid);
				}
			}
			// TODO check if buffer destruction goes correctly
		}
	}


	void Material::CreateDescriptorSets()
	{
		m_descriptorSets = JoyContext::DescriptorSet->Allocate(
			m_sharedMaterial->GetSetLayoutHash(),
			JoyContext::Render->GetSwapchain()->GetSwapchainImageCount());

		std::vector<VkWriteDescriptorSet> descriptorWrites(m_descriptorSets.size());

		for (int i = 0; i < m_bindings.size(); i++)
		{
			VkDescriptorImageInfo* imageInfoPtr = nullptr;
			VkDescriptorBufferInfo* bufferInfoPtr = nullptr;
			VkBufferView* texelBufferViewPtr = nullptr;

			VkDescriptorImageInfo imageInfo = {};
			VkDescriptorBufferInfo bufferInfo = {};
			VkBufferView texelBufferView = {};

			for (int j = 0; j < m_descriptorSets.size(); j++)
			{
				switch (m_bindings[i].bindingDescription.type)
				{
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					{
						Texture* texture = m_bindings[i].textureGuid.IsNull()
							                   ? JoyContext::DescriptorSet->GetTexture()
							                   : JoyContext::Resource->GetResource<Texture>(m_bindings[i].textureGuid);
						imageInfo = {
							texture->GetSampler(),
							texture->GetImageView(),
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
						};
						imageInfoPtr = &imageInfo;
						break;
					}
				case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
					{
						Texture* texture = nullptr;
						switch (m_bindings[i].inputAttachmentType)
						{
						case Position:
							texture = JoyContext::Render->GetGBufferPositionTexture();
							break;
						case Normal:
							texture = JoyContext::Render->GetGBufferNormalTexture();
							break;
						default:
							ASSERT(false);
						}
						imageInfo = {
							texture->GetSampler(),
							texture->GetImageView(),
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
						};
						imageInfoPtr = &imageInfo;
						break;
					}
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					{
						bufferInfo = {
							m_bindings[i].buffers[j]->GetBuffer(),
							0,
							m_bindings[i].bindingDescription.size,

						};
						bufferInfoPtr = &bufferInfo;
						break;
					}
				default:
					ASSERT(false);
					break;
				}
				descriptorWrites[j] =
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					m_descriptorSets[j],
					static_cast<uint32_t>(i),
					0,
					1,
					m_bindings[i].bindingDescription.type,
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

	SharedMaterial* Material::GetSharedMaterial() const noexcept
	{
		return m_sharedMaterial;
	}

	std::vector<VkDescriptorSet>& Material::GetDescriptorSets() noexcept
	{
		return m_descriptorSets;
	}

	bool Material::IsLoaded() const noexcept
	{
		if (!m_sharedMaterial->IsLoaded()) return false;

		for (const auto& item : m_bindings)
		{
			if (item.bindingDescription.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (!item.textureGuid.IsNull())
				{
					const bool textureIsReady = JoyContext::Resource->GetResource<Texture>(item.textureGuid)->
					                                                  IsLoaded();
					if (!textureIsReady) return false;
				}
			}
			// binding buffers are host visible so data uploading is not async
		}
		return true;
	}
}
