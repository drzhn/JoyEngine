#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <filesystem>

#include "Utils/FileUtils.h"
#include "Utils/GUID.h"


namespace JoyEngine {
    class DataManager {
    public:
        DataManager();

        ~DataManager();

        static DataManager *GetInstance() noexcept {
            ASSERT(m_instance != nullptr);
            return m_instance;
        }

        template<typename T>
        std::vector<T> GetData(GUID guid) {
            if (m_pathDatabase.find(guid) == m_pathDatabase.end()) {
                ASSERT(false);
            }
            return readFile<T>(m_dataPath + m_pathDatabase[guid].string());
        }

        void GetDataStream(std::ifstream &, GUID);

        void ParseMaterial(const GUID &materialGuid, GUID &sharedMaterialGuid, std::map<std::string, GUID> &bindings);

        void ParseSharedMaterial(const GUID &sharedMaterialGuid,
                                 GUID &vertexShader,
                                 GUID &fragmentShader,
                                 bool &hasVertexInput,
                                 bool &hasMVP,
                                 bool &depthTest,
                                 bool &depthWrite,
                                 std::vector<std::vector<std::tuple<std::string, std::string>>> &bindingSets
        );

    private:
        static DataManager *m_instance;

        const std::string m_dataPath = R"(D:\CppProjects\JoyEngine\JoyData\)";
        const std::string m_databaseFilename = R"(data.db)";
        std::map<GUID, std::filesystem::path> m_pathDatabase;

    private:
        void ParseDatabase(std::map<GUID, std::filesystem::path> &pathDatabase, const char *data);

        const std::filesystem::path &GetPath(GUID);
    };
}

#endif //DATA_MANAGER_H
