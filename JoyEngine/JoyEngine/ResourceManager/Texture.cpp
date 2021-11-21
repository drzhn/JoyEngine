#include "Texture.h"

#include "JoyContext.h"

#include <vector>

#include "Utils/Assert.h"

#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"
#include "GraphicsManager/GraphicsManager.h"


namespace JoyEngine
{
	std::string ParseVkResult(VkResult res);

	Texture::Texture() :
		m_width(1),
		m_height(1),
		m_format(VK_FORMAT_R8G8B8A8_SRGB),
		m_tiling(VK_IMAGE_TILING_OPTIMAL),
		m_usageFlags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		m_propertiesFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		m_aspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
	{
		constexpr unsigned char pixels[] = {255, 255, 255, 255};
		InitializeTexture(pixels);
	}

	Texture::Texture(GUID guid) :
		Resource(guid),
		m_format(VK_FORMAT_R8G8B8A8_SRGB), // TODO get texture format from texture data
		m_tiling(VK_IMAGE_TILING_OPTIMAL),
		m_usageFlags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		m_propertiesFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		m_aspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
	{
		m_textureStream = JoyContext::Data->GetFileStream(guid, true);
		uint32_t width, height;

		m_textureStream.seekg(0);
		m_textureStream.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
		m_textureStream.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));


		m_width = width;
		m_height = height;


		InitializeTexture(m_textureStream, sizeof(uint32_t) + sizeof(uint32_t));
	}

	void Texture::InitializeTexture(const unsigned char* data)
	{
		CreateImage();
		CreateImageView();
		CreateImageSampler();

		JoyContext::Memory->LoadDataToImage(data, m_width, m_height, m_textureImage);
		m_isLoaded = true;
	}

	void Texture::InitializeTexture(std::ifstream& stream, uint64_t offset)
	{
		CreateImage();
		CreateImageView();
		CreateImageSampler();

		JoyContext::Memory->LoadDataToImageAsync(stream, offset, m_width, m_height, m_textureImage, m_onLoadedCallback);
	}

	Texture::Texture(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags) :
		m_width(width),
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
			JoyContext::Graphics->GetDevice(),
			&imageInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_textureImage);

		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(JoyContext::Graphics->GetDevice(), m_textureImage, &memRequirements);

		JoyContext::Memory->AllocateMemory(memRequirements, m_propertiesFlags, m_textureImageMemory);

		res = vkBindImageMemory(JoyContext::Graphics->GetDevice(), m_textureImage, m_textureImageMemory, 0);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	void Texture::CreateImageView()
	{
		m_subresourceRange = {
				m_aspectFlags,
				0,
				1,
				0,
				1
		};

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
			m_subresourceRange
		};

		const VkResult res = vkCreateImageView(
			JoyContext::Graphics->GetDevice(),
			&viewInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_textureImageView);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	void Texture::CreateImageSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(JoyContext::Graphics->GetPhysicalDevice(), &properties);

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
			JoyContext::Graphics->GetDevice(),
			&samplerInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_textureSampler);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}


	Texture::~Texture()
	{
		if (m_textureSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(JoyContext::Graphics->GetDevice(), m_textureSampler,
			                 JoyContext::Graphics->GetAllocationCallbacks());
		}
		vkDestroyImageView(JoyContext::Graphics->GetDevice(), m_textureImageView,
		                   JoyContext::Graphics->GetAllocationCallbacks());
		vkDestroyImage(JoyContext::Graphics->GetDevice(), m_textureImage,
		               JoyContext::Graphics->GetAllocationCallbacks());
		vkFreeMemory(JoyContext::Graphics->GetDevice(), m_textureImageMemory,
		             JoyContext::Graphics->GetAllocationCallbacks());
	}
}
