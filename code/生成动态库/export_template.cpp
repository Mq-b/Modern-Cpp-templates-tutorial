#include "export_template.h"

template<typename T>
void f(T) {
    std::cout << typeid(T).name() << '\n';
}