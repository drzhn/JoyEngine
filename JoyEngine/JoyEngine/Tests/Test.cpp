//constexpr uint32_t hashQuickConstexpr(const char *s, uint32_t hash = 5381) noexcept {
//    return !*s ? hash : hashQuickConstexpr(s + 1, uint32_t(hash * uint64_t(33) ^ *s));
//}
//
//#define REFLECTABLE(T) constexpr uint32_t T##_typeId = hashQuickConstexpr(#T); class T
//#define typeId(T) T##_typeId
//#define HASH(T) hashQuickConstexpr(#T)

#include <iostream>
#include "Common/Serialization.h"

namespace JoyEngine {

}

class A {
public:
    A() { std::cout << "Foo From A"; };
};

//class B : public A {
//public:
//    void Foo() override { std::cout << "Foo From B"; };
//};

static A a;
int main() {
    return 0;
}