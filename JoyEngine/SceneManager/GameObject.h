#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <string>
#include <vector>
#include <type_traits>

#include "Transform.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"

namespace JoyEngine {
    class GameObject {
    public:
        explicit GameObject() : GameObject("GameObject") {}

        explicit GameObject(const char *name) {
            m_name = name;
            m_transform = Transform();
        }

        Transform *GetTransform() { return &m_transform; }

        MeshRenderer *AddMeshRenderer() {
            MeshRenderer *mr = new MeshRenderer(m_transform);
            m_components.push_back(mr);
            return mr;
        }

    private:
        Transform m_transform;
        std::string m_name;
        std::vector<Component *> m_components;
    };
}


#endif //GAME_OBJECT_H
