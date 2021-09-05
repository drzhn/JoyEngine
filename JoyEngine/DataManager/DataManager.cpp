#include "DataManager.h"
#include <fstream>
#include <iostream>

#include "Utils/ObjectParser.h"
#include "Utils/FileUtils.h"

namespace JoyEngine {

    DataManager *DataManager::m_instance = nullptr;

    DataManager::DataManager() {
        ObjectParser::ParseDatabase(m_pathDatabase, readFile(m_dataPath + m_databaseFilename).data());
    }

    DataManager::~DataManager() {

    }

    const std::filesystem::path &DataManager::GetPath(GUID guid) {
        if (m_pathDatabase.find(guid) == m_pathDatabase.end()) {
            ASSERT(false);
        }
        return m_pathDatabase[guid];
    }

    std::vector<char> DataManager::GetData(GUID guid) {
        if (m_pathDatabase.find(guid) == m_pathDatabase.end()) {
            ASSERT(false);
        }
        return readFile(m_dataPath + m_pathDatabase[guid].string());
    }

    void DataManager::GetDataStream(std::ifstream &stream, GUID guid) {
        getFileStream(stream, GetPath(guid).string());
    }
}