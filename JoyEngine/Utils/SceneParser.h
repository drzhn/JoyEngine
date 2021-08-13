#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <string>
#include <iostream>
#include <Libs/glm/glm/glm.hpp>

#include "FileUtils.h"
#include "GUID.h"
#include "SceneManager/GameObject.h"
#include "Components/MeshRenderer.h"
#include "rapidjson/document.h"

namespace JoyEngine {


    class SceneParser {
    public:
        SceneParser(const char *filename) {
            std::vector<char> json = JoyEngine::readFile(filename);
            m_document.Parse<rapidjson::kParseStopWhenDoneFlag>(json.data());
        }

        std::string GetName() {
            rapidjson::Value &val = m_document["name"];
            return val.GetString();
        }

        std::vector<GameObject *> GetObjects() {
            rapidjson::Value &val = m_document["objects"];
            std::vector<GameObject *> objects;
            for (auto &v : val.GetArray()) {
                GameObject *go = new GameObject(v["name"].GetString());

                rapidjson::Value &transformValue = v["transform"];
                go->GetTransform()->SetPosition(GetVectorValueFromField(transformValue, "localPosition"));
                go->GetTransform()->SetRotation(GetVectorValueFromField(transformValue, "localRotation"));
                go->GetTransform()->SetScale(GetVectorValueFromField(transformValue, "localScale"));

                for (auto &c : v["components"].GetArray()) {
                    if (std::string(c["type"].GetString()) == "renderer") {
                        MeshRenderer *mr = go->AddMeshRenderer();
                        mr->SetMesh(c["model"]["path"].GetString(), GUID::StringToGuid(c["model"]["fileId"].GetString()));
                        mr->SetTexture(c["texture"]["path"].GetString(), GUID::StringToGuid(c["model"]["fileId"].GetString()));
                        mr->SetVertShader(c["vertexShader"]["path"].GetString(), GUID::StringToGuid(c["model"]["fileId"].GetString()));
                        mr->SetFragShader(c["fragmentShader"]["path"].GetString(), GUID::StringToGuid(c["model"]["fileId"].GetString()));
                    }

                }

                objects.push_back(go);
            }
            return objects;
        }

        static glm::vec3 GetVectorValueFromField(rapidjson::Value &val, const char *name) {
            return {val[name]["x"].GetFloat(), val[name]["y"].GetFloat(), val[name]["z"].GetFloat()};
        }

    private:
        rapidjson::Document m_document;
    };
}

#endif //SCENE_PARSER_H
