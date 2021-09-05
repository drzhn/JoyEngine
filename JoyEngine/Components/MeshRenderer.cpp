#include "MeshRenderer.h"

#include "RenderManager/RenderManager.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine {
    MeshRenderer::MeshRenderer(Transform t) : m_transform(t),
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

        }
        m_meshGuid = meshGuid;
    }

    void MeshRenderer::SetMaterial(GUID materialGuid) {
        if (m_materialGuid.has_value()) {

        }
        m_materialGuid = materialGuid;
    }
}
