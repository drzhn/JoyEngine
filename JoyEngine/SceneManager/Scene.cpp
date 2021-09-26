#include "Scene.h"

#include "rapidjson/document.h"

#include "JoyContext.h"
#include "Common/Serialization.h"
#include "Components/Component.h"
#include "DataManager/DataManager.h"
//#include "GameplayComponents/RoomBehaviour.h"

namespace JoyEngine {
    Scene::Scene(const GUID &guid) {
        //Goo();

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
                if (std::string(component["type"].GetString()) == "component") {
                    ASSERT(component.HasMember("component"));
                    auto type = component["component"].GetString();
                    ASSERT(SerializableClassFactory::GetInstance() != nullptr);
                    std::unique_ptr<Serializable> s = SerializableClassFactory::GetInstance()->Deserialize(component["fields"], component["component"].GetString());
                    auto *c_ptr = dynamic_cast<Component *>(s.release());
                    ASSERT(c_ptr != nullptr);
                    std::unique_ptr<Component> c(c_ptr);
                    go->AddComponent(std::move(c));
                }
            }
            m_objects.push_back(std::move(go));
        }
    }

}