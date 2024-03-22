#include "test_function_template.h"
#include "test_class_template.h"

int main() {
    f_t(1);
    f_t(1.2);
    f_t('c');
    //f_t("1");   // 没有显式实例化 f_t<const char*> 版本，会有链接错误

    N::X<int>x;
    x.f();
    //x.f2();     // 链接错误，没有显式实例化 X<int>::f2() 成员函数
    N::X<double>x2{};
    //x2.f();     // 链接错误，没有显式实例化 X<double>::f() 成员函数

    N::X2<int>x3; // 我们显式实例化了类模板 X2<int> 也就自然而然实例化它所有的成员，f，f2 函数
    x3.f();
    x3.f2();

    // 类模板分文件 我们写了两个类模板 X X2，它们一个使用了成员函数显式实例化，一个类模板显式实例化，进行对比
    // 这主要在于我们所谓的类模板分文件，其实类模板定义还是在头文件中，只不过成员函数定义在 cpp 罢了。
}