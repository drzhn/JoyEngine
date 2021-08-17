#include "MeshRenderer.h"

#include "RenderManager/RenderManager.h"

namespace JoyEngine {
    void MeshRenderer::Enable() {
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
        delete m_mesh;
        delete m_texture;
        delete m_vertShader;
        delete m_fragShader;
    }
}
