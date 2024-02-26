## 折叠表达式作业

题目如下：

```cpp
template<class ...Args>
auto Reverse(Args&&... args) {
    std::vector<std::common_type_t<Args...>> res{};
    bool tmp{ false };
    (tmp = ... = (res.push_back(args), false));
    return res;
}
```

首先**看函数名也知道是逆序传入的参数包**，主要是分析代码。

---

考虑以下调用：

```cpp
auto vec = Reverse(1, 2, 3, 4.);
for (const auto n : vec) {
    std::cout << n << ' ';
}
```

对于

```cpp
std::vector<std::common_type_t<Args...>> res{};
```

`std::common_type_t` 用于寻找传入类型的公共类型，这里传入的是 3个 int 以及 1个 double 类型参数，故 res 的类型是 `std::vector<double>`

对于

```cpp
bool tmp{ false };
(tmp = ... = (res.push_back(args), false));
```

首先定义一个 bool 变量，单纯用于后续包展开。

观察折叠表达式，按照**国王的教诲**，观察到 `= ... =` ，其次 `...` 在参数包 `args` 左侧，故该表达式是**二元左折叠表达式**。

展开后大致为：

```cpp
((((tmp = (res.push_back(1), false)) = (res.push_back(2), false)) = (res.push_back(3), false)) = (res.push_back(4.), false));
```

考虑 [求值顺序](https://en.cppreference.com/w/cpp/language/eval_order)，

对于 `,` 运算符，从左到右求值，这里只是 `push_back(arg)` 并且返回 false 给左侧整体进行赋值。

对于 `=` 运算符，右侧的值计算和副作用先于左侧。所以该表达式整体也从右侧开始执行，按传入的参数包逆序 `push_back` 入 `res`。

最后的输出是：

```
4 3 2 1
```
