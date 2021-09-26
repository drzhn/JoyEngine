#include "RoomBehaviour.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

#include "Common/Time.h"

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
		m_transform->SetRotation(
			glm::angleAxis(Time::GetTime(), glm::vec3(0, 1, 0)) *
			glm::angleAxis(glm::pi<float>()/2, glm::vec3(-1, 0, 0)));
	}

}

