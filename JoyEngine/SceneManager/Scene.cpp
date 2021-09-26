#include "Scene.h"

#include "rapidjson/document.h"

#include "JoyContext.h"
#include "Common/Serialization.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "DataManager/DataManager.h"

namespace JoyEngine {
    Scene::Scene(const GUID &guid) {

        rapidjson::Document json = JoyContext::Data()->GetSerializedData(guid, scene);
        m_name = json["name"].GetString();

        auto GetVectorValueFromField = [](rapidjson::Value& val, const char* name) -> glm::vec3 {
            float x = val[name]["x"].GetFloat();
            float y = val[name]["y"].GetFloat();
            float z = val[name]["z"].GetFloat();
            return glm::vec3(x,y,z);
        };

        rapidjson::Value &val = json["objects"];
        for (auto &obj: val.GetArray()) {
            std::unique_ptr<GameObject> go = std::make_unique<GameObject>(obj["name"].GetString());

            rapidjson::Value &transformValue = obj["transform"];
            go->GetTransform()->SetPosition(GetVectorValueFromField(transformValue, "localPosition"));
            go->GetTransform()->SetRotation(GetVectorValueFromField(transformValue, "localRotation"));
            go->GetTransform()->SetScale(GetVectorValueFromField(transformValue, "localScale"));

            for (auto &component: obj["components"].GetArray()) {
                std::string type = std::string(component["type"].GetString());
                if (type == "renderer") {
                    std::unique_ptr<MeshRenderer> mr = std::make_unique<MeshRenderer>();
                    mr->SetMesh(GUID::StringToGuid(component["model"].GetString()));
                    mr->SetMaterial(GUID::StringToGuid(component["material"].GetString()));
                    go->AddComponent(std::move(mr));
                }
                else if (type == "component") {
                    ASSERT(component.HasMember("component"));
                    auto type = component["component"].GetString();
                    ASSERT(SerializableClassFactory::GetInstance() != nullptr);
                    std::unique_ptr<Serializable> s = SerializableClassFactory::GetInstance()->Deserialize(component["fields"], component["component"].GetString());
                    auto *c_ptr = dynamic_cast<Component *>(s.release());
                    ASSERT(c_ptr != nullptr);
                    std::unique_ptr<Component> c(c_ptr);
                    go->AddComponent(std::move(c));
                }
                else if (type == "camera") {
                    go->AddComponent(std::move(std::make_unique<Camera>()));
                }
            }
            m_objects.push_back(std::move(go));
        }
    }

    void Scene::Update()
    {
        for (const auto& o : m_objects) {
            o->Update();
        }
    }

}