#ifndef ASSERT_H
#define ASSERT_H

#ifdef DEBUG

#include <iostream>

#ifdef _MSC_VER
#include <intrin.h>
#define DEBUG_BREAK __debugbreak()
#else
#endif

#define ASSERT(expr) if (expr) {} else { std::cerr << #expr << " " <<  __FILE__ <<  ":" << __LINE__ << std::endl; DEBUG_BREAK;}
#define ASSERT_DESC(expr, message) if (expr) {} else { std::cerr << message << " " << #expr << " " <<  __FILE__ <<  ":" << __LINE__ << std::endl; DEBUG_BREAK;}

#else
#define ASSERT(expr)
#define ASSERT_DESC(expr, message)
#endif //DEBUG

#endif //ASSERT_H
