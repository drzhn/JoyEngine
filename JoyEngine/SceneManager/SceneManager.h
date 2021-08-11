#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <map>

//#include "ResourceManager/ResourceManager.h"

#include "Scene.h"
#include "Utils/GUIDUtils.h"

namespace JoyEngine {
//    class ResourceManager;

    class SceneManager {
    public:
        SceneManager() = default;

//        explicit SceneManager(ResourceManager *resourceManager) :
//                m_resourceManager(resourceManager),
//                m_scene(nullptr) {}

        void Init() {
            LoadScene(R"(D:\CppProjects\JoyEngine\JoyData\scenes\room.json)");
        }

        void Start() {}

        void Stop() {}

        void LoadScene(const char *filename);

        void UnloadScene();

        ~SceneManager() {
            if (m_scene != nullptr) {
                UnloadScene();
            }
            delete m_scene;
        }

    private:
//        const ResourceManager *m_resourceManager;
        Scene *m_scene;
    };
}

#endif //SCENE_MANAGER_H