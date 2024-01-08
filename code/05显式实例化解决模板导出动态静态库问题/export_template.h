#pragma once

#include <iostream>
#include <string>

template<typename T>
void f(T);

template __declspec(dllexport)  void f<int>(int);
template __declspec(dllexport)  void f<std::string>(std::string);

template<typename T>
struct __declspec(dllexport) X {
    void f() { std::cout << typeid(T).name() << '\n'; }
};

template struct X<int>;