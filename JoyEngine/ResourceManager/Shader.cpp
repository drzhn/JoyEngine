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
		const std::vector<char> shaderData = JoyContext::Data->GetData<char>(guid);

		VkShaderModuleCreateInfo createInfo{
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			nullptr,
			0,
			shaderData.size(),
			reinterpret_cast<const uint32_t*>(shaderData.data())
		};
		const VkResult res = vkCreateShaderModule(
			JoyContext::Graphics->GetVkDevice(),
			&createInfo,
			JoyContext::Graphics->GetAllocationCallbacks(),
			&m_shaderModule);
		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(
			JoyContext::Graphics->GetVkDevice(), m_shaderModule,
			JoyContext::Graphics->GetAllocationCallbacks());
	}
}
