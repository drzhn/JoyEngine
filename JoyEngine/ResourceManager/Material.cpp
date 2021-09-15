#include "Material.h"

#include "JoyContext.h"

#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
    Material::Material(GUID guid) {
        JoyContext::Data()->ParseMaterial(guid, m_sharedMaterialGuid, m_bindings);
        JoyContext::Resource()->LoadResource<SharedMaterial>(m_sharedMaterialGuid);
    }

    Material::~Material() {
        JoyContext::Resource()->UnloadResource(m_sharedMaterialGuid);
        for (const auto &item: m_bindings) {
            JoyContext::Resource()->UnloadResource(item.second);
        }
    }

    SharedMaterial *Material::GetSharedMaterial() const noexcept {
        return JoyContext::Resource()->GetResource<SharedMaterial>(m_sharedMaterialGuid);
    }
}