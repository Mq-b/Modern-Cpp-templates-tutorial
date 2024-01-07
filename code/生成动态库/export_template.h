#pragma once

#include <iostream>
#include <string>

template<typename T>
void f(T);

template __declspec(dllexport)  void f<int>(int);
template __declspec(dllexport)  void f<std::string>(std::string);