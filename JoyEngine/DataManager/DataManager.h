#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <filesystem>

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

        const std::filesystem::path& GetPath(GUID);
        std::vector<char> GetData(GUID);

    private:
        static DataManager *m_instance;

        const std::string m_dataPath = R"(D:\CppProjects\JoyEngine\JoyData\)";
        const std::string m_databaseFilename = R"(data.db)";
        std::map<GUID, std::filesystem::path> m_pathDatabase;
    };
}

#endif //DATA_MANAGER_H
