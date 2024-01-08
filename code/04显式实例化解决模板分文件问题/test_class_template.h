#pragma once

#include <iostream>
#include <typeinfo>

namespace N {

    template<typename T>
    struct X {
        int a{};
        void f();
        void f2();
    };

    template<typename T>
    struct X2 {
        int a{};
        void f();
        void f2();
    };
};