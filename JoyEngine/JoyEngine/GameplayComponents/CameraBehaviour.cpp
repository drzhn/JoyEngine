#include "CameraBehaviour.h"

#include "JoyContext.h"
#include "Common/Time.h"
#include "InputManager/InputManager.h"

namespace JoyEngine
{
	DECLARE_CLASS(CameraBehaviour)

	void CameraBehaviour::Enable()
	{
		m_enabled = true;
	}

	void CameraBehaviour::Disable()
	{
		m_enabled = false;
	}

	void CameraBehaviour::Update()
	{
		glm::vec3 vec = glm::vec3(
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_A) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_D) ? Time::GetDeltaTime() : 0),
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_E) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_Q) ? Time::GetDeltaTime() : 0),
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_W) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_S) ? Time::GetDeltaTime() : 0));

		m_transform->SetPosition(
			m_transform->GetPosition() + vec
		);
		//m_transform->SetRotation(
		//	glm::angleAxis(Time::GetTime() * m_speed, glm::vec3(0, 1, 0)) *
		//	glm::angleAxis(glm::pi<float>() / 2, glm::vec3(-1, 0, 0)));
	}
}
