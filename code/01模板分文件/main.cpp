#include "test.h"
#include "test_template.h"

int main(){
    f();    // 非模板，OK
    f_t(1); // 模板 链接错误
}