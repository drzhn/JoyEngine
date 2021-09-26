#include "RoomBehaviour.h"
#include <iostream>

namespace JoyEngine {

//    DECLARE_CLASS(RoomBehaviour)

    static SerializedObjectCreator<RoomBehaviour> RoomBehaviour_creator = SerializedObjectCreator<RoomBehaviour>("RoomBehaviour");


    void RoomBehaviour::Enable() {
        m_enabled = true;
    }

    void RoomBehaviour::Disable() {
        m_enabled = false;
    }

    void Goo() {

    }
}

