#include "Scene.h"

#include "JoyContext.h"
#include "rapidjson/document.h"
#include "DataManager/DataManager.h"

#include <glm/glm.hpp>

namespace JoyEngine {
    Scene::Scene(const GUID &guid) {
        rapidjson::Document json = JoyContext::Data()->GetSerializedData(guid, scene);
        m_name = json["name"].GetString();

        auto GetVectorValueFromField = [](rapidjson::Value &val, const char *name) -> glm::vec3 {
            return {val[name]["x"].GetFloat(), val[name]["y"].GetFloat(), val[name]["z"].GetFloat()};
        };

        rapidjson::Value &val = json["objects"];
        for (auto &obj: val.GetArray()) {
            std::unique_ptr<GameObject> go = std::make_unique<GameObject>(obj["name"].GetString());

            rapidjson::Value &transformValue = obj["transform"];
            go->GetTransform()->SetPosition(GetVectorValueFromField(transformValue, "localPosition"));
            go->GetTransform()->SetRotation(GetVectorValueFromField(transformValue, "localRotation"));
            go->GetTransform()->SetScale(GetVectorValueFromField(transformValue, "localScale"));

            for (auto &component: obj["components"].GetArray()) {
                if (std::string(component["type"].GetString()) == "renderer") {
                    go->AddMeshRenderer(GUID::StringToGuid(component["model"].GetString()),
                                        GUID::StringToGuid(component["material"].GetString()));
                }

            }
            m_objects.push_back(std::move(go));
        }
    }

}