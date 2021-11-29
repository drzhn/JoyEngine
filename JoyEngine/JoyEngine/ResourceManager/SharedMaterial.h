#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include <map>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "Common/Resource.h"
#include "Shader.h"
#include "Texture.h"
#include "Utils/GUID.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	class MeshRenderer;

	enum ShaderDefineType
	{
		JOY_VARIABLES,
		GBUFFER_TEXTURES
	};

	struct VulkanBindingDescription
	{
		VkDescriptorType type;
		size_t size;
	};

	struct BindingInfo
	{
		uint32_t bindingIndex;
		std::string type;
		uint32_t count;
		size_t offset;
	};

	struct BindingBase
	{
		VkDescriptorType type;

		// buffers are not shared, who creates them is responsible for destruction
		std::vector<std::unique_ptr<Buffer>> buffers;
		// for attachment binding
		Texture* texture = nullptr;
		// textures are shared resources, so we need to create them through ResourceManagers
		GUID textureGuid;
	};

	class SharedMaterial final : public Resource
	{
	public :
		SharedMaterial() = delete;

		explicit SharedMaterial(GUID);

		~SharedMaterial() final;

		[[nodiscard]] VkPipeline GetPipeline() const noexcept;

		[[nodiscard]] VkPipelineLayout GetPipelineLayout() const noexcept;
		[[nodiscard]] BindingInfo* GetBindingInfoByName(const std::string& name) noexcept;
		[[nodiscard]] uint64_t GetSetLayoutHash() const noexcept;
		[[nodiscard]] std::vector<VulkanBindingDescription>& GetVulkanBindings();

		static VkDescriptorType GetTypeFromStr(const std::string& type) noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;

		void RegisterMeshRenderer(MeshRenderer* meshRenderer);

		void UnregisterMeshRenderer(MeshRenderer* meshRenderer);

		std::set<MeshRenderer*>& GetMeshRenderers();
		std::vector<uint32_t>& GetBindingDefines();

	private :
		std::set<MeshRenderer*> m_meshRenderers;

		ResourceHandle<Shader> m_shader;

		bool m_hasVertexInput = false;
		bool m_hasMVP = false;
		bool m_depthTest = false;
		bool m_depthWrite = false;
		uint32_t m_colorAttachmentsCount;
		uint32_t m_subpassIndex;

		VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
		uint64_t m_setLayoutHash;
		std::map<std::string, BindingInfo> m_bindings;
		std::vector<VulkanBindingDescription> m_vulkanBindings;

		std::vector<uint32_t> m_bindingDefines;

		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

	private:
		void Initialize();

		void CreateGraphicsPipeline();
	};
}

#endif //SHARED_MATERIAL_H
