#include "RoomBehaviour.h"
#include <iostream>

namespace JoyEngine {

//    DECLARE_CLASS(RoomBehaviour)

    SerializedObjectCreator<RoomBehaviour> RoomBehaviour_creator = SerializedObjectCreator<RoomBehaviour>("RoomBehaviour");

    int Foo() {
        std::cout << "sdfsdf";
        return 56;
    }


    void RoomBehaviour::Enable() {
        m_enabled = true;
    }

    void RoomBehaviour::Disable() {
        m_enabled = false;
    }

}

