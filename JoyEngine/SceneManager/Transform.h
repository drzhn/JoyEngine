#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <list>

#include <Libs/glm/glm/glm.hpp>

class Transform {
public :
    Transform() :
            m_localPosition(glm::vec3(0, 0, 0)),
            m_localRotation(glm::vec3(0, 0, 0)),
            m_localScale(glm::vec3(1, 1, 1)) {
    }

    void SetPosition(glm::vec3 pos) noexcept { m_localPosition = pos; };

    [[nodiscard]] glm::vec3 GetPosition() const noexcept { return m_localPosition; }

    void SetRotation(glm::vec3 pos) noexcept { m_localRotation = pos; };

    [[nodiscard]] glm::vec3 GetRotation() const noexcept { return m_localRotation; }

    void SetScale(glm::vec3 pos) noexcept { m_localScale = pos; };

    [[nodiscard]] glm::vec3 GetScale() const noexcept { return m_localScale; }

private:
    glm::vec3 m_localPosition;
    glm::vec3 m_localRotation;
    glm::vec3 m_localScale;
};


#endif //TRANSFORM_H
