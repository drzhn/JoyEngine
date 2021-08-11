//#ifndef ASSERT_H
//#define ASSERT_H
//
//#include <iostream>
//
////#if ASSERTIONS_ENABLED
//// define some inline assembly that causes a break
//// into the debugger -- this will be different on each
//// target CPU
////#define debugBreak() asm { int 3 }
//void debugBreak() {
//    __asm  {int 3}
//}
//
//void checkExpr(bool expr)
//{
//    if (expr) {} else { std::cerr << expr <<  __FILE__ <<  __LINE__ << std::endl; debugBreak();}
//}
//// check the expression and fail if it is false
////#define ASSERT(expr) if (expr) {} else { std::cerr << #expr <<  __FILE__ <<  __LINE__ << std::endl; debugBreak();}
//#define ASSERT(expr) checkExpr(expr);
////#else
////#define ASSERT(expr) // evaluates to nothing
////#endif
//
//#endif //ASSERT_H
