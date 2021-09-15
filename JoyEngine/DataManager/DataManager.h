#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <filesystem>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"
#include "Utils/GUID.h"


namespace JoyEngine {
    enum DataType {
        mesh,
        texture,
        shader,
        material,
        sharedMaterial
    };

    class DataManager {
    public:
        DataManager();

        ~DataManager();

        template<typename T>
        std::vector<T> GetData(GUID guid) {
            if (m_pathDatabase.find(guid) == m_pathDatabase.end()) {
                ASSERT(false);
            }
            return readFile<T>(m_dataPath + m_pathDatabase[guid].string());
        }

        void GetDataStream(std::ifstream &, GUID);

        void ParseMaterial(const GUID &materialGuid, GUID &sharedMaterialGuid, std::map<std::string, GUID> &bindings);

        rapidjson::Document GetSerializedData(const GUID &, DataType);

    private:
        const std::string m_dataPath = R"(D:\CppProjects\JoyEngine\JoyData\)";
        const std::string m_databaseFilename = R"(data.db)";
        std::map<GUID, std::filesystem::path> m_pathDatabase;

    private:
        void ParseDatabase(std::map<GUID, std::filesystem::path> &pathDatabase, const char *data);

        const std::filesystem::path &GetPath(GUID);
    };
}

#endif //DATA_MANAGER_H
