#ifndef COMMON_DESCRIPTOR_SET_PROVIDER_H
#define COMMON_DESCRIPTOR_SET_PROVIDER_H

#include <map>

#include "Common/HashDefs.h"
#include "Components/Camera.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	struct SharedBindingData
	{
		uint32_t setIndex;
		VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
		uint64_t setLayoutHash;
		std::vector<BindingBase> m_bindings;
		std::vector<VkDescriptorSet> descriptorSets;

		~SharedBindingData();
	};

	class CommonDescriptorSetProvider
	{
	public:
		CommonDescriptorSetProvider();
		void SetCamera(Camera* camera);
		void CreateDescriptorSets();
		void UpdateDescriptorSetData(uint32_t imageIndex) const;
		[[nodiscard]] SharedBindingData* GetBindingData(uint32_t defineHash);
		~CommonDescriptorSetProvider();
	private:
		static constexpr const char* JoyVariablesStr = "JOY_VARIABLES";
		static constexpr const char* GBufferTexturesStr = "GBUFFER_TEXTURES";

		Camera* m_camera = nullptr;
		std::map<uint32_t, std::unique_ptr<SharedBindingData>> m_data;
	};
}

#endif // COMMON_DESCRIPTOR_SET_PROVIDER_H
