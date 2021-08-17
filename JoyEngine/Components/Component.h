#ifndef COMPONENT_H
#define COMPONENT_H

namespace JoyEngine {
    class Component {
    public:
        virtual void Enable() = 0;

        virtual void Disable() = 0;

        bool IsEnabled() const noexcept { return m_enabled; }

    protected:
        bool m_enabled = false;
    };
}


#endif //COMPONENT_H
