<!--
 * @Description: 
 * @Author: lize
 * @Date: 2024-03-12
 * @LastEditors: lize
-->
### 题目

```c++
template<class ...Args>
auto Reverse(Args&&... args) {
    std::vector<std::common_type_t<Args...>> res{};
    bool tmp{ false };
    (tmp = ... = (res.push_back(args), false));
    return res;
}
```

### 作用

返回一个包含参数包逆序的vector

### 解析

+ ...在参数包左边 所以是一个 **左折叠**，其中 ```= ... =``` 符合二元的形式， 所以是一个**二元左折叠**。

+ 我们知道逗号表达式的值是**最右侧**子表达式的值. 所以```(res.push_back(args), false))```值一定是false。

+ 所以套一下公式```((((I 运算符 E1) 运算符 E2) 运算符 ...) 运算符 EN)```.展开如下:

```
((((false = E1) = E2) = ...) = EN) // 实际EX的值都是false
```

+ 我们知道赋值运算符的执行顺序是**从右向左**，所以按EN...E2,E1的顺序依次执行。自然就会将参数包中传入参数的逆序返回。
