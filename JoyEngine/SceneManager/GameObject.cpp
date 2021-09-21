#include "GameObject.h"

namespace JoyEngine {
    void GameObject::AddMeshRenderer(GUID meshGuid, GUID materialGuid) { // TODO Remove?
        std::unique_ptr<MeshRenderer> mr = std::make_unique<MeshRenderer>(&m_transform);
        mr->SetMesh(meshGuid);
        mr->SetMaterial(materialGuid);
        mr->Enable();
        m_components.push_back(std::move(mr));
    }

    void GameObject::AddComponent(std::unique_ptr<Component> component) {
        m_components.push_back(std::move(component));
    }
}
