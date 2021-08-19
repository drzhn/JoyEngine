#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <map>
#include <memory>

//#include "ResourceManager/ResourceManager.h"

#include "Scene.h"
#include "Utils/GUID.h"

namespace JoyEngine {
//    class ResourceManager;

    class SceneManager {
    public:
        SceneManager() = default;

//        explicit SceneManager(ResourceManager *resourceManager) :
//                m_resourceManager(resourceManager),
//                m_scene(nullptr) {}

        void Init() {
            m_scene = std::make_unique<Scene>(R"(D:\CppProjects\JoyEngine\JoyData\scenes\room.json)");
        }

        void Start() {

        }

        void Stop() {
            m_scene = nullptr;
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