#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <map>
#include <memory>


#include "Scene.h"
#include "Utils/GUID.h"

namespace JoyEngine {
//    class ResourceManager;

    class SceneManager {
    public:
        SceneManager() = default;

        void Init() {
            m_scene = std::make_unique<Scene>(GUID::StringToGuid("11dcfeba-c2b6-4c2e-a3c7-51054ff06f1d"));
        }

        void Start() {

        }

        void Stop() {
            m_scene = nullptr;
        }

        void Update() {
            m_scene->Update();
        }

        ~SceneManager() {
            if (m_scene != nullptr) {
                m_scene = nullptr;
            }
        }

    private:
        std::unique_ptr<Scene> m_scene = nullptr;
    };
}

#endif //SCENE_MANAGER_H