#include "export_template.h"
#include <string>


int main(){
    std::string s;
    f(1);
    //f(1.2); // Error!链接错误，没有这个符号
    f(s);
    X<int>x;
    x.f();
}