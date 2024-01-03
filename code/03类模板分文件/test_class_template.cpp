#include "test_class_template.h"

template <typename T>
void X2<T>::f(){
    std::cout << "X2<T>::f\n";
}