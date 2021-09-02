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


#include "Utils/Assert.h"

int main()
{
    ASSERT(false);
}
