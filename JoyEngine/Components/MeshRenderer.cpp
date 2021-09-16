#include "MeshRenderer.h"

#include "JoyContext.h"

#include "RenderManager/RenderManager.h"
#include "Common/Resource.h"

namespace JoyEngine {
    MeshRenderer::MeshRenderer(Transform *t) : Component(t){

    }

    void MeshRenderer::Enable() {
        ASSERT(m_meshGuid.has_value() && m_materialGuid.has_value());
        JoyContext::Render()->RegisterMeshRenderer(this);
        m_enabled = true;
    }

    void MeshRenderer::Disable() {
        JoyContext::Render()->UnregisterMeshRenderer(this);
        m_enabled = false;
    }

    MeshRenderer::~MeshRenderer() {
        if (m_enabled) {
            Disable();
        }
    }

    void MeshRenderer::SetMesh(GUID meshGuid) {
        if (m_meshGuid.has_value()) {
            JoyContext::Resource()->UnloadResource(meshGuid);
        }
        m_meshGuid = meshGuid;
        JoyContext::Resource()->LoadResource<Mesh>(meshGuid);
    }

    void MeshRenderer::SetMaterial(GUID materialGuid) {
        if (m_materialGuid.has_value()) {
            JoyContext::Resource()->UnloadResource(materialGuid);
        }
        m_materialGuid = materialGuid;
        JoyContext::Resource()->LoadResource<Material>(materialGuid);
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
        return JoyContext::Resource()->GetResource<Mesh>(m_meshGuid.value());
    }

    Material *MeshRenderer::GetMaterial() const noexcept {
        return JoyContext::Resource()->GetResource<Material>(m_materialGuid.value());
    }
}
