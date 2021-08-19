#include "Scene.h"

#include "Utils/SceneParser.h"

namespace JoyEngine {
    Scene::Scene(const std::string &filename) {
        SceneParser parser(filename);
        m_name = parser.GetName();
        m_objects = parser.GetObjects();
    };
}