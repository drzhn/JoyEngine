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

#include <iostream>

class A {
public:
    A() {
        std::cout << "A created" << std::endl;
    }

    virtual ~A() {
        std::cout << "A destroyed" << std::endl;

    }
};

class B : public A {
public:
    B() {
        std::cout << "B created" << std::endl;
    }

    ~B() {
        std::cout << "B destroyed" << std::endl;
    }
};

int main() {
    A a = B();
}
