#include "Shader.h"

#include "JoyContext.h"

#include <vector>

#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	std::string ParseVkResult(VkResult res);

	Shader::Shader(GUID guid) : Resource(guid)
	{
		const std::vector<char> shaderData = JoyContext::Data->GetData(guid, true);
		uint32_t vertexDataLength = 0;
		memcpy(&vertexDataLength, shaderData.data(), sizeof(uint32_t));
		uint32_t fragmentDataLength = 0;
		memcpy(&fragmentDataLength, shaderData.data() + sizeof(uint32_t), sizeof(uint32_t));

		VkShaderModuleCreateInfo createInfo;
		VkResult res;

		createInfo = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			nullptr,
			0,
			vertexDataLength,
			reinterpret_cast<const uint32_t*>(shaderData.data() + sizeof(uint32_t) * 2)
		};
		res = vkCreateShaderModule(
			JoyContext::Graphics->GetDevice(),
			&createInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_vertexModule);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));

		createInfo = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			nullptr,
			0,
			fragmentDataLength,
			reinterpret_cast<const uint32_t*>(shaderData.data() + sizeof(uint32_t) * 2 + vertexDataLength)
		};
		res = vkCreateShaderModule(
			JoyContext::Graphics->GetDevice(),
			&createInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_fragmentModule);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(
			JoyContext::Graphics->GetDevice(), 
			m_vertexModule,
			JoyContext::Graphics->GetAllocationCallbacks());

		vkDestroyShaderModule(
			JoyContext::Graphics->GetDevice(),
			m_fragmentModule,
			JoyContext::Graphics->GetAllocationCallbacks());
	}
}
