#ifndef COMPONENT_H
#define COMPONENT_H

#include "SceneManager/Transform.h"
#include "Common/Serializable.h"

namespace JoyEngine {
    class Component : public Serializable {
    public:
        Component() = default;

        explicit Component(Transform *t);

        ~Component() override = default;

        virtual void Enable() = 0;

        virtual void Disable() = 0;

        bool IsEnabled() const noexcept { return m_enabled; }

    protected:
        Transform const *m_transform;

        bool m_enabled = false;
    };
}


#endif //COMPONENT_H
