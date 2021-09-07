#include "Material.h"

#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
    Material::Material(GUID guid) {
        DataManager::GetInstance()->ParseMaterial(guid, m_sharedMaterialGuid, m_bindings);
        ResourceManager::GetInstance()->LoadResource<SharedMaterial>(m_sharedMaterialGuid);
    }

    Material::~Material() {
        ResourceManager::GetInstance()->UnloadResource(m_sharedMaterialGuid);
        for (const auto &item: m_bindings) {
            ResourceManager::GetInstance()->UnloadResource(item.second);
        }
    }

    SharedMaterial *Material::GetSharedMaterial() const noexcept {
        return ResourceManager::GetInstance()->GetResource<SharedMaterial>(m_sharedMaterialGuid);
    }
}