#include "DataManager.h"
#include <fstream>
#include <iostream>

#include "Utils/ObjectParser.h"
#include "Utils/FileUtils.h"

namespace JoyEngine {

    DataManager::DataManager() {
        // walk recursively in data path, search json, make guid->file database;
        // yes, I understand it is stupid
        // TODO cache in some DB like sqlite
//        for (const std::filesystem::directory_entry &p: std::filesystem::recursive_directory_iterator(m_dataPath)) {
//            if (!p.is_directory()) {
//                if (p.path().extension() == ".json")
//                {
//                    std::cout << p.path() << '\n';
//                }
//            }
//        }
        ObjectParser::ParseDatabase(m_pathDatabase, readFile(m_dataPath + m_databaseFilename).data());
    }

    DataManager::~DataManager() {

    }
}