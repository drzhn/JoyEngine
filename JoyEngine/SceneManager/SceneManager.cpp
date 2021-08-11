#include "SceneManager.h"

#include "Utils/SceneParser.h"

namespace JoyEngine {
    void SceneManager::LoadScene(const char *filename) {
        SceneParser parser(filename);
        m_scene = new Scene(parser.GetName());
        m_scene->SetGameObjects(parser.GetObjects());
    }

    void SceneManager::UnloadScene() {

    }

}