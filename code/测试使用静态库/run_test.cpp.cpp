#include "export_template.h"
#include <string>

#pragma comment(lib, "生成静态库.lib")

int main(){
    std::string s;
    f(1);
    //f(1.2); // Error!链接错误，没有这个符号
    f(s);
    X<int>x;
    x.f();
}