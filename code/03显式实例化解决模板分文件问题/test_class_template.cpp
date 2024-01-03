#include "test_class_template.h"

template<typename T>
void N::X<T>::f(){
    std::cout << "f: " << typeid(T).name() << '\n';
}

template void N::X<int>::f();    // 显式实例化定义