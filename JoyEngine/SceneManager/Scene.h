#ifndef SCENE_H
#define SCENE_H

#include <utility>
#include <vector>
#include <string>

#include "GameObject.h"

namespace JoyEngine {
    class Scene {
    public :
        Scene(const std::string &name);

//        void AddGameObject(GameObject *go) {
//            m_objects.push_back(go);
//        }

        void Update() {
            // Update GameObjects throw logic in components;
        }

    private:
        std::string m_name;
        std::vector<GameObject *> m_objects;
    };
}

#endif //SCENE_H