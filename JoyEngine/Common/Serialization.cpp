#include "Serialization.h"

namespace JoyEngine {


    SerializableClassFactory *SerializableClassFactory::m_instance = nullptr;

    void SerializableClassFactory::RegisterClass(const std::string &className, SerializedObjectCreatorBase *creator) {
        GetInstance()->m_classCreatorStorage.insert(std::make_pair(className, creator));
    }

    void SerializableClassFactory::RegisterClassFieldOffset(const std::string &className, const std::string &filedName, FieldInfo fieldInfo) {
        if (GetInstance()->m_fieldOffsetStorage.find(className) == GetInstance()->m_fieldOffsetStorage.end()) {
            GetInstance()->m_fieldOffsetStorage.insert({className, std::map<std::string, FieldInfo>()});
        }
        if (GetInstance()->m_fieldOffsetStorage[className].find(filedName) == GetInstance()->m_fieldOffsetStorage[className].end()) {
            GetInstance()->m_fieldOffsetStorage[className].insert({filedName, fieldInfo});
        } else {
            GetInstance()->m_fieldOffsetStorage[className][filedName].fieldOffset = fieldInfo.fieldOffset;
        }
    }

    std::unique_ptr<Serializable> SerializableClassFactory::Deserialize(rapidjson::Value &value, const std::string &className) {
        ASSERT(GetInstance()->m_classCreatorStorage.find(className) != GetInstance()->m_classCreatorStorage.end());
        SerializedObjectCreatorBase *creator = GetInstance()->m_classCreatorStorage.find(className)->second;
        std::unique_ptr<Serializable> object = creator->Create();
        auto fieldOffsetStorage = GetInstance()->m_fieldOffsetStorage[className];
        for (auto member = value.MemberBegin(); member != value.MemberEnd(); member++) {
            const std::string fieldName = member->name.GetString();
            uint32_t typeHash = fieldOffsetStorage[fieldName].typeHash;
            void *fieldOffset = fieldOffsetStorage[fieldName].fieldOffset;
            rapidjson::Value &val = member->value;
            switch (typeHash) {
                case HASH(float): {
                    float f = value.GetFloat();
                    memcpy(fieldOffset, &f, sizeof(float));
                    break;
                }
                case HASH(int32_t): {
                    int32_t f = value.GetInt();
                    memcpy(fieldOffset, &f, sizeof(int32_t));
                    break;
                }
                case HASH(uint32_t): {
                    uint32_t f = value.GetUint();
                    memcpy(fieldOffset, &f, sizeof(uint32_t));
                    break;
                }
                case HASH(Color): {
                    Color color = Color(value["r"].GetFloat(), value["g"].GetFloat(), value["b"].GetFloat(), value["a"].GetFloat());
                    memcpy(fieldOffset, &color, sizeof(Color));
                }
                case HASH(glm::vec2): {
                    glm::vec2 vec = glm::vec2(value["x"].GetFloat(), value["y"].GetFloat());
                    memcpy(fieldOffset, &vec, sizeof(glm::vec2));
                }
                case HASH(glm::vec3): {
                    glm::vec3 vec = glm::vec3(value["x"].GetFloat(), value["y"].GetFloat(), value["z"].GetFloat());
                    memcpy(fieldOffset, &vec, sizeof(glm::vec3));
                }
                case HASH(Serializable): {
                    ASSERT(false); // TODO later :)
                }
                default:
                    ASSERT(false);
            }
        }
        return std::move(object);
    }
}
