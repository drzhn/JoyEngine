#include "MeshRenderer.h"

#include "RenderManager/RenderManager.h"
#include "ResourceManager/Resource.h"

namespace JoyEngine {
    MeshRenderer::MeshRenderer(Transform *t) : Component(t),
                                               m_index(UINT32_MAX) {

    }

    void MeshRenderer::Enable() {
        ASSERT(m_meshGuid.has_value() && m_materialGuid.has_value());
        m_index = RenderManager::GetInstance()->RegisterMeshRenderer(this);
        m_enabled = true;
    }

    void MeshRenderer::Disable() {
        RenderManager::GetInstance()->UnregisterMeshRenderer(m_index);
        m_enabled = false;
    }

    MeshRenderer::~MeshRenderer() {
        if (m_enabled) {
            Disable();
        }
    }

    void MeshRenderer::SetMesh(GUID meshGuid) {
        if (m_meshGuid.has_value()) {
            ResourceManager::GetInstance()->UnloadResource(meshGuid);
        }
        m_meshGuid = meshGuid;
        ResourceManager::GetInstance()->LoadResource<Mesh>(meshGuid);
    }

    void MeshRenderer::SetMaterial(GUID materialGuid) {
        if (m_materialGuid.has_value()) {
            ResourceManager::GetInstance()->UnloadResource(materialGuid);
        }
        m_materialGuid = materialGuid;
        ResourceManager::GetInstance()->LoadResource<Material>(materialGuid);
    }

    GUID MeshRenderer::GetMeshGuid() const noexcept {
        ASSERT(m_meshGuid.has_value());
        return m_meshGuid.value();
    }

    GUID MeshRenderer::GetMaterialGuid() const noexcept {
        ASSERT(m_materialGuid.has_value());
        return m_materialGuid.value();
    }

    Mesh *MeshRenderer::GetMesh() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Mesh>(m_meshGuid.value());
    }

    Material *MeshRenderer::GetMaterial() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Material>(m_materialGuid.value());
    }
}
