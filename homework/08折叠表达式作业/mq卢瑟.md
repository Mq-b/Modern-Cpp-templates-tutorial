mq卢瑟的作业如下：

首先作业题目是： 

```c++
// 说出以下代码使用的折叠表达式语法，以及它的效果，详细解析，使用 Markdown 语法。
template<class ...Args>
auto Reverse(Args&&... args) {
    std::vector<std::common_type_t<Args...>> res{};
    bool tmp{ false };
    (tmp = ... = (res.push_back(args), false));
    return res;
}
```

假设 main 函数按照下面方式调用 Reverse 函数： 

```c++
auto arr = Reverse(1.1, 3);
```
---

对于代码 `std::vector<std::common_type_t<Args...>> res{};` ，解释如下：

> `01函数模板.md` 中讲过" std::common_type_t 的作用很简单，就是确定我们传入的共用类型，说白了就是这些东西都能隐式转换到哪个，那就会返回那个类型。"
> 所以，根据我main函数中传入的"1.1,3"，这里res的类型是 `std::vector<double>`

对于代码 `(tmp = ... = (res.push_back(args), false));` ，解释如下： 

... 在 形参包左侧，所以这是一个二元左折叠表达式，根据下面的展开方式： 

`((((I 运算符 E1) 运算符 E2) 运算符 ...) 运算符 EN)` 

实例化展开后是这样： 

```c++
std::vector<double> Reverse(double && args0, int && args1)
{
	std::vector<double> res = std::vector<double>{};
	bool tmp = {false};
  	(tmp = (res.push_back(args0) , false)) = (res.push_back(args1) , false);
  	return res;
}
```

又因为 loserhomework 的卢瑟日经 [内建赋值运算符求值问题](https://github.com/Mq-b/Loser-HomeWork/blob/main/src/%E5%8D%A2%E7%91%9F%E6%97%A5%E7%BB%8F/%E8%B5%8B%E5%80%BC%E8%BF%90%E7%AE%97%E7%AC%A6%E6%B1%82%E5%80%BC%E9%A1%BA%E5%BA%8F%E9%97%AE%E9%A2%98.md) 中提到的：
> 每个简单赋值表达式 E1 = E2 和每个复合赋值表达式 E1 @= E2 中，E2 的每个值计算和副作用都按顺序早于 E1 的每个值计算和副作用。

因此， args1 会先被 push_back 到 res 这个容器中，所以容器中的元素顺序是：3 , 1.1。

***所以该函数是将传入的参数进行逆序***
