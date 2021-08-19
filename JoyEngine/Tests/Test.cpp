
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

#include <memory>

class Data {
public :
    Data(int a) :
            m_a(a) {
        std::cout << " data constructed " << m_a << std::endl;
    }

    ~Data() {
        std::cout << " data destructed " << m_a << std::endl;
    }

    int m_a;
};

class Consumer {
public:
    Consumer(std::unique_ptr<Data>& data) :
            m_data(data) {
        m_data = nullptr;
    }

    std::unique_ptr<Data> &m_data;
};

class Manager {
public:
    Manager() {
        data1 = std::make_unique<Data>(1);
        data2 = std::make_unique<Data>(2);
        Consumer *a = new Consumer(data1);
    }

    ~Manager() {
        data2 = nullptr;
        data1 = nullptr;
    }

    std::unique_ptr<Data> data1;
    std::unique_ptr<Data> data2;
};

int main() {
    Manager m;
    return 0;
}
