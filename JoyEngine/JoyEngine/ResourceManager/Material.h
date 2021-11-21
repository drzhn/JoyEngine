#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>
#include <memory>

#include "Common/Resource.h"
#include "SharedMaterial.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	enum InputAttachmentType
	{
		Position,
		Normal
	};

	struct Binding
	{
		VulkanBindingDescription bindingDescription;

		// buffers are not shared, who creates them is responsible for destruction
		std::vector<std::unique_ptr<Buffer>> buffers;
		// textures are shared resources, so we need to create them through ResourceManagers
		GUID textureGuid;
		InputAttachmentType inputAttachmentType;
	};

	class Material final : public Resource
	{
	public :
		Material() = delete;

		explicit Material(GUID);

		~Material() final;

		[[nodiscard]] SharedMaterial* GetSharedMaterial() const noexcept;

		[[nodiscard]] std::vector<VkDescriptorSet>& GetDescriptorSets() noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;
	private:
		void CreateDescriptorSets();

		void LoadResources() const;

	private :
		GUID m_sharedMaterialGuid;
		SharedMaterial* m_sharedMaterial = nullptr;
		std::vector<Binding> m_bindings;
		std::vector<VkDescriptorSet> m_descriptorSets;
	};
}

#endif //MATERIAL_H
