#include"test_function_template.h"

template<typename T>
void f_t(T) { std::cout << typeid(T).name() << '\n'; }

template void f_t<int>(int); // 显式实例化定义 实例化 f_t<int>(int)
template void f_t<>(char);   // 显式实例化定义 实例化 f_t<char>(char)，推导出模板实参
template void f_t(double);   // 显式实例化定义 实例化 f_t<double>(double)，推导出模板实参