#include "SharedMaterial.h"

#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine {
    SharedMaterial::SharedMaterial(GUID guid) {
        DataManager::GetInstance()->ParseSharedMaterial(guid,
                                                        m_vertexShader,
                                                        m_fragmentShader,
                                                        m_hasVertexInput,
                                                        m_hasMVP,
                                                        m_depthTest,
                                                        m_depthWrite,
                                                        m_bindingSets
        );
    }

    SharedMaterial::~SharedMaterial() {
        ResourceManager::GetInstance()->UnloadResource(m_vertexShader);
        ResourceManager::GetInstance()->UnloadResource(m_fragmentShader);
    }

    Shader *SharedMaterial::GetVertexShader() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Shader>(m_vertexShader);
    }

    Shader *SharedMaterial::GetFragmentShader() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Shader>(m_fragmentShader);
    }
}