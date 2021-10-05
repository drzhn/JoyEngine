#include "Texture.h"

#include "JoyContext.h"

#include <vector>

#include "Utils/Assert.h"

#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"
#include "GraphicsManager/GraphicsManager.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

namespace JoyEngine
{
	std::string ParseVkResult(VkResult res);

	Texture::Texture(GUID guid) : Resource(guid)
	{
		const std::vector<unsigned char> imageData = JoyContext::Data->GetData<unsigned char>(guid);
		int texWidth, texHeight, texChannels;

		stbi_uc* pixels = stbi_load_from_memory(
			imageData.data(),
			static_cast<int>(imageData.size()),
			&texWidth,
			&texHeight, &texChannels, STBI_rgb_alpha);

		ASSERT(pixels != nullptr);

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		m_width = texWidth;
		m_height = texHeight;
		m_format = VK_FORMAT_R8G8B8A8_SRGB;
		m_tiling = VK_IMAGE_TILING_OPTIMAL;
		m_usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		m_propertiesFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		m_aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		CreateImage();
		CreateImageView();
		CreateImageSampler();

		JoyContext::Memory->LoadDataToImage(pixels, texWidth, texHeight, m_textureImage);

		stbi_image_free(pixels);

	}

	Texture::Texture(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags) : m_width(width),
		                                  m_height(height),
		                                  m_format(format),
		                                  m_tiling(tiling),
		                                  m_usageFlags(usage),
		                                  m_propertiesFlags(properties),
		                                  m_aspectFlags(aspectFlags)

	{
		CreateImage();
		CreateImageView();
	}

	void Texture::CreateImage()
	{
		const VkImageCreateInfo imageInfo{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			nullptr,
			0,
			VK_IMAGE_TYPE_2D,
			m_format,
			{m_width, m_height, 1},
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			m_tiling,
			m_usageFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			VK_IMAGE_LAYOUT_UNDEFINED
		};


		VkResult res = vkCreateImage(
			JoyContext::Graphics->GetVkDevice(),
			&imageInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_textureImage);

		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(JoyContext::Graphics->GetVkDevice(), m_textureImage, &memRequirements);

		JoyContext::Memory->AllocateMemory(memRequirements, m_propertiesFlags, m_textureImageMemory);

		res = vkBindImageMemory(JoyContext::Graphics->GetVkDevice(), m_textureImage, m_textureImageMemory, 0);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	void Texture::CreateImageView()
	{
		const VkImageViewCreateInfo viewInfo{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			m_textureImage,
			VK_IMAGE_VIEW_TYPE_2D,
			m_format,
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			{
				m_aspectFlags,
				0,
				1,
				0,
				1
			}
		};

		const VkResult res = vkCreateImageView(
			JoyContext::Graphics->GetVkDevice(),
			&viewInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_textureImageView);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	void Texture::CreateImageSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(JoyContext::Graphics->GetVkPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_TRUE,
			properties.limits.maxSamplerAnisotropy,
			VK_FALSE,
			VK_COMPARE_OP_ALWAYS,
			0.0f,
			0.0f,
			VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			VK_FALSE
		};

		const VkResult res = vkCreateSampler(
			JoyContext::Graphics->GetVkDevice(),
			&samplerInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_textureSampler);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}


	Texture::~Texture()
	{
		if (m_textureSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(JoyContext::Graphics->GetVkDevice(), m_textureSampler,
			                 JoyContext::Graphics->GetAllocationCallbacks());
		}
		vkDestroyImageView(JoyContext::Graphics->GetVkDevice(), m_textureImageView,
		                   JoyContext::Graphics->GetAllocationCallbacks());
		vkDestroyImage(JoyContext::Graphics->GetVkDevice(), m_textureImage,
		               JoyContext::Graphics->GetAllocationCallbacks());
		vkFreeMemory(JoyContext::Graphics->GetVkDevice(), m_textureImageMemory,
		             JoyContext::Graphics->GetAllocationCallbacks());
	}
}
