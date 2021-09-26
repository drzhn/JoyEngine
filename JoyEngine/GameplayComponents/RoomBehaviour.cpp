#include "RoomBehaviour.h"
#include <iostream>

namespace JoyEngine {

    DECLARE_CLASS(RoomBehaviour)


    void RoomBehaviour::Enable() {
        m_enabled = true;
    }

    void RoomBehaviour::Disable() {
        m_enabled = false;
    }

    void RoomBehaviour::Update()
    {

    }

}

