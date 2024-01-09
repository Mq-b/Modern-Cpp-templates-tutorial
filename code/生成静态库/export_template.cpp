#include "export_template.h"

template<typename T>
void f(T) {
    std::cout << typeid(T).name() << '\n';
}

template <typename T>
void X<T>::f(){
    std::cout << typeid(T).name() << '\n';
}

template void f<int>(int);
template void f<std::string>(std::string);

template struct X<int>;