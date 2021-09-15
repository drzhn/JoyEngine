#include "DataManager.h"
#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"

namespace JoyEngine {

    DataManager::DataManager() {
        ParseDatabase(m_pathDatabase, readFile<char>(m_dataPath + m_databaseFilename).data());
    }

    DataManager::~DataManager() {

    }

    const std::filesystem::path &DataManager::GetPath(GUID guid) {
        if (m_pathDatabase.find(guid) == m_pathDatabase.end()) {
            ASSERT(false);
        }
        return m_pathDatabase[guid];
    }

    void DataManager::GetDataStream(std::ifstream &stream, GUID guid) {
        getFileStream(stream, GetPath(guid).string());
    }

    void DataManager::ParseDatabase(std::map<GUID, std::filesystem::path> &pathDatabase, const char *data) {
        rapidjson::Document json;
        json.Parse<rapidjson::kParseStopWhenDoneFlag>(data);
        ASSERT(json["type"].GetString() == std::string("database"));
        rapidjson::Value &val = json["database"];
        for (auto &v: val.GetArray()) {
            pathDatabase.insert({
                                        GUID::StringToGuid(v["guid"].GetString()),
                                        v["path"].GetString()
                                });
        }
    }

    void DataManager::ParseMaterial(const GUID &materialGuid, GUID &sharedMaterialGuid, std::map<std::string, GUID> &bindings) {
        std::vector<char> data = GetData<char>(materialGuid);
        rapidjson::Document json;
        json.Parse<rapidjson::kParseStopWhenDoneFlag>(data.data());
        ASSERT(json["type"].GetString() == std::string("material"));
        sharedMaterialGuid = GUID::StringToGuid(json["sharedMaterial"].GetString());

        for (auto &binding: json["bindings"].GetArray()) {
            bindings.insert({
                                    binding["name"].GetString(),
                                    GUID::StringToGuid(binding["data"].GetString())
                                    // TODO What about another types of bindings? int, float, color, array, etc...
                            });
        }
    }

    rapidjson::Document DataManager::GetSerializedData(const GUID &sharedMaterialGuid, DataType type) {
        std::vector<char> data = GetData<char>(sharedMaterialGuid);
        rapidjson::Document json;
        json.Parse<rapidjson::kParseStopWhenDoneFlag>(data.data());
        std::string s;
        switch (type) {
            case mesh:
                s = "mesh";
            case texture:
                s = "texture";
            case shader:
                s = "shader";
            case material:
                s = "material";
            case sharedMaterial:
                s = "sharedMaterial";
            default:
                ASSERT(false);
        }
        ASSERT(json.HasMember("type") && json["type"].GetString() == std::string(s));
        return json;
    }
}