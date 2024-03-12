# 原题：

```
template<class ...Args>
auto Reverse(Args&&... args) {
    std::vector<std::common_type_t<Args...>> res{};
    bool tmp{ false };
    (tmp = ... = (res.push_back(args), false));
    return res;
}
```

## 解释：

首先看`Reverse(Args&&... args)`    名为`Reverse`的模板函数接受可变参数列表，可以接受任意个参数

`std::vector<std::common_type_t<Args...>> res{};`  common_type_t确定所有参数的公共类型，将所有的参数转化为同一类型。用这个类型初始化了一个空的vector容器。

`bool tmp{ false };`  初始化一个值为false的bool变量，后面用它来作为二元折叠的一部分

`(tmp = ... = (res.push_back(args), false));` 这是一个二元左折叠，将每个args放入容器中

> 这里的折叠展开来应当是：
>
> tmp=(res.push_back(arg0),flase)=(res.push_back(arg1),flase)...=(res.push_back(argn),flase)

逗号运算符返回最后一个表达式，这里的flase应该是作为表达式的值，实际上没什么用，只是为了用这个折叠表达式。

`return res;`  最后将容器返回；
