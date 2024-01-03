#include "test_function_template.h"
#include "test_class_template.h"

int main() {
    f_t(1);
    f_t(1.2);
    f_t('c');
    //f_t("1"); // 没有显式实例化 f_t<const char*> 版本，会有链接错误

    N::X<int>x;
    x.f();
    N::X<double>x2{};
    //x2.f();   // 链接错误，没有显式实例化 X<double>::f()

    // 类模板分文件 没有使用到类模板显式实例化，而是使用了函数模板显式实例化。
    // 这主要在于我们所谓的类模板分文件，其实类模板定义还是在头文件中，只不过成员函数定义在cpp罢了。
}