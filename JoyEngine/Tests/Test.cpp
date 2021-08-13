
#include <iostream>

//#include "Render/GpuAllocator.h"


//constexpr uint32_t hashQuickConstexpr(const char *s, uint32_t hash = 5381) noexcept {
//    return !*s ? hash : hashQuickConstexpr(s + 1, uint32_t(hash * uint64_t(33) ^ *s));
//}
//
//#define reflectable_class(T) constexpr uint32_t T##_typeId = hashQuickConstexpr(#T); class T
//#define typeId(T) T##_typeId
//
//reflectable_class(Example) {
//public:
//    int a = 0;
//};
//
//
//
//int main() {
//    Example a;
//    std::cout << typeId(Example);
////    JoyEngine::GPUMemoryArea area(1, 8);
////    uint32_t offset;
////    area.Allocate(1, offset);
////    std::cout << offset;
////
////    area.Allocate(2, offset);
////    std::cout << offset;
////
////    area.Allocate(3, offset);
////    std::cout << offset;
//
////    std::cout << 1 << " " << nearPow(1) << std::endl;
////    std::cout << 2 << " " << nearPow(2) << std::endl;
////    std::cout << 3 << " " << nearPow(3) << std::endl;
////    std::cout << 4 << " " << nearPow(4) << std::endl;
////    std::cout << 5 << " " << nearPow(5) << std::endl;
////    std::cout << 6 << " " << nearPow(6) << std::endl;
////    std::cout << 7 << " " << nearPow(7) << std::endl;
////    std::cout << 8 << " " << nearPow(8) << std::endl;
////    std::cout << 9 << " " << nearPow(9) << std::endl;
////    std::cout << 10 << " " << nearPow(10) << std::endl;
//    return 0;
//}


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <guiddef.h>
#include <windows.h>
#include <Combaseapi.h>
#include "Utils/FileUtils.h"
#include"Utils/GUID.h"
#include <map>
#include <unordered_map>

using namespace rapidjson;

const char *filename = "../../../JoyData/scenes/room.json";

int main() {
//    GUID gidReference;
//    HRESULT hCreateGuid = CoCreateGuid( &gidReference );
//    size_t b= sizeof(unsigned int  );
    // 1. Parse a JSON string into DOM.
//    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
//    std::vector<char> json = JoyEngine::readFile(filename);
//    Document document;
//    document.Parse<rapidjson::kParseStopWhenDoneFlag>(json.data());
//
//    Value &val = document["guid"];
//    const char* guidStr = val.GetString();
//    JoyEngine::GUID guid = JoyEngine::GUID::StringToGuid(guidStr);
//    std::map<JoyEngine::GUID, int> map;
//    map.insert({guid,4});

    // 2. Modify it by DOM.
//    Value &s = document["stars"];
//    s.SetInt(s.GetInt() + 1);

    // 3. Stringify the DOM
//    StringBuffer buffer;
//    Writer<StringBuffer> writer(buffer);
//    document.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
//    std::cout << buffer.GetString() << std::endl;
    return 0;
}
