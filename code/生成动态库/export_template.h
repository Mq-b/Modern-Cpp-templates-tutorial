#pragma once

#include <iostream>
#include <string>

template<typename T>
void f(T);

template<typename T>
struct __declspec(dllexport) X {
    void f();
};