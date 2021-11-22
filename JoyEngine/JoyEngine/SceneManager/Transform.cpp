#include "Transform.h"

namespace JoyEngine
{
	Transform::Transform() : Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1))
	{
	}

	Transform::Transform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		SetPosition(pos);
		SetRotation(rot);
		SetScale(scale);
	}

	void Transform::SetPosition(glm::vec3 pos) noexcept
	{
		m_localPosition = pos;
	}

	glm::vec3 Transform::GetPosition() const noexcept { return m_localPosition; }

	void Transform::SetRotation(glm::vec3 rot) noexcept
	{
		m_localRotation = glm::quat(glm::vec3(
			glm::radians(rot.x),
			glm::radians(rot.y),
			glm::radians(rot.z)
		));
	}

	void Transform::SetRotation(glm::quat rot) noexcept
	{
		m_localRotation = rot;
	}

	glm::quat Transform::GetRotation() const noexcept { return m_localRotation; }

	void Transform::SetScale(glm::vec3 scale) noexcept { m_localScale = scale; }

	glm::vec3 Transform::GetScale() const noexcept { return m_localScale; }

	glm::mat4 Transform::GetModelMatrix()
	{
		const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_localPosition);
		const glm::mat4 rotationMatrix = glm::toMat4(m_localRotation);
		const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_localScale);

		return translationMatrix * rotationMatrix * scaleMatrix;
	}
}
