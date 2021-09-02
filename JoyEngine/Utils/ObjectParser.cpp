#include "ObjectParser.h"

#include <string>

#include "rapidjson/document.h"
#include "Utils/Assert.h"


namespace JoyEngine {
    void ObjectParser::ParseDatabase(std::map<GUID, std::filesystem::path> &pathDatabase, const char *data) {
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
}