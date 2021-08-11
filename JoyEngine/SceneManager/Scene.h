#ifndef SCENE_H
#define SCENE_H

#include <utility>
#include <vector>
#include <string>

#include "GameObject.h"

namespace JoyEngine {
    class Scene {
    public :
        Scene(std::string name) : m_name(std::move(name)) {};

        void AddGameObject(GameObject *go) {
            m_objects.push_back(go);
        }

        void Update() {
            // Update GameObjects throw logic in components;
        }

        void SetGameObjects(std::vector<GameObject *> objects) {
            m_objects = objects;
        }

    private:
        std::string m_name;
        std::vector<GameObject *> m_objects;
    };
}

#endif //SCENE_H