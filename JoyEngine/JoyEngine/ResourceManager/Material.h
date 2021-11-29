#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>
#include <memory>

#include "Common/Resource.h"
#include "SharedMaterial.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/ResourceHandle.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	enum InputAttachmentType
	{
		Position,
		Normal
	};

	struct MaterialBinding : BindingBase
	{
		size_t size;
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
	private :
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::vector<MaterialBinding> m_bindings;
		std::vector<VkDescriptorSet> m_descriptorSets;
	};
}

#endif //MATERIAL_H
