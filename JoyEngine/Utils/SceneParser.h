#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <string>
#include <iostream>
#include <memory>
#include <Libs/glm/glm/glm.hpp>

#include "FileUtils.h"
#include "GUID.h"
#include "SceneManager/GameObject.h"
#include "Components/MeshRenderer.h"
#include "rapidjson/document.h"

namespace JoyEngine {
    class SceneParser {
    public:
        SceneParser(const std::string &filename) {
            std::vector<char> json = readFile(filename);
            m_document.Parse<rapidjson::kParseStopWhenDoneFlag>(json.data());
        }

        std::string GetName() {
            rapidjson::Value &val = m_document["name"];
            return val.GetString();
        }

        void GetObjects(std::vector<std::unique_ptr<GameObject>> &objects) {
            rapidjson::Value &val = m_document["objects"];
            for (auto &v: val.GetArray()) {
                std::unique_ptr<GameObject> go = std::make_unique<GameObject>(v["name"].GetString());

                rapidjson::Value &transformValue = v["transform"];
                go->GetTransform()->SetPosition(GetVectorValueFromField(transformValue, "localPosition"));
                go->GetTransform()->SetRotation(GetVectorValueFromField(transformValue, "localRotation"));
                go->GetTransform()->SetScale(GetVectorValueFromField(transformValue, "localScale"));

                for (auto &component: v["components"].GetArray()) {
                    if (std::string(component["type"].GetString()) == "renderer") {
                        go->AddMeshRenderer(GUID::StringToGuid(component["model"].GetString()),
                                            GUID::StringToGuid(component["material"].GetString()));
                    }

                }
                objects.push_back(std::move(go));
            }
        }

        static glm::vec3 GetVectorValueFromField(rapidjson::Value &val, const char *name) {
            return {val[name]["x"].GetFloat(), val[name]["y"].GetFloat(), val[name]["z"].GetFloat()};
        }

    private:
        rapidjson::Document m_document;
        const std::string dataPath = R"(D:\CppProjects\JoyEngine\JoyData\)";
    };
}

#endif //SCENE_PARSER_H
