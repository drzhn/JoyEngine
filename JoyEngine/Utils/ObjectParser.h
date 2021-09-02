//
// Temporary solution for parsing json objects from disk since we don't have any serialization library yet
//

#ifndef OBJECT_PARSER_H
#define OBJECT_PARSER_H

#include <map>
#include <filesystem>

#include "Utils/GUID.h"

namespace JoyEngine {
    class ObjectParser {
    public:
        static void ParseDatabase(std::map<GUID, std::filesystem::path> &pathDatabase, const char *data);
    };
}

#endif //OBJECT_PARSER_H
