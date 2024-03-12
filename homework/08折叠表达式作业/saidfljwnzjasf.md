### 题目 
>说出以下代码使用的折叠表达式语法，以及它的效果，详细解析，使用 Markdown 语法。
>```cpp
>template<class ...Args>
>auto Reverse(Args&&... args) {
>    std::vector<std::common_type_t<Args...>> res{};
>    bool tmp{ false };
>    (tmp = ... = (res.push_back(args), false));
>    return res;
>}
>```

### 解析
首先我们找到折叠表达式所在的语句
```cpp
(tmp = ... = (res.push_back(args), false));
```
这里表达式符合 `( 初值 运算符 ... 运算符 形参包 )` 这个形式，属于二元左折叠。
那么表达式会被展开成下面这种形式的语句：
```cpp
((((tmp = (res.push_back(args[0]), false)) = (res.push_back(args[1]), false)) = (...)) = (res.push_back(args[N-1]), false)) = (res.push_back(args[N]), false);
```
现在我们知道展开后的表达式语句长什么样子了，但是这个语句是如何先让 `args[N]` 被 push_back 到 `res` 中呢？  
因为这里的 `=` 是一个简单赋值，又因为：
>每个简单赋值表达式 E1 = E2 和每个复合赋值表达式 E1 @= E2 中，E2 的每个值计算和副作用都按顺序早于 E1 的每个值计算和副作用。  

所以这里会先追加 `args[N]` 到 `res` 容器尾。  
其实这里我不确定是不是这个原因，因为在 cppreference 里又提到：
>对于优先级相同的运算符：
>
>拥有相同优先级的运算符以其结合性的方向与各参数绑定。例如表达式 `a = b = c` 会被分析为 `a = (b = c)` 而非 `(a = b) = c`，因为赋值具有从右到左结合性

而语句中的 `,` 运算符被包裹在`()`中以保证优先级高于 `=` 运算符，并且让作为左实参的 `std::vector::push_back` 调用作为弃值表达式来实施它的副作用，让 `flase` 完成一个合法的简单赋值。

用 C++ Insights [验证](https://cppinsights.io/s/1b5d6eab)一下我们的想法。在 C++ Insights 中我们得到了如下结果：
>```cpp
>template<>
>std::vector<int, std::allocator<int> > Reverse<int, int, int, int>(int && __args0, int && __args1, int && __args2, int && __args3)
>{
>  std::vector<int, std::allocator<int> > res = std::vector<int, std::allocator<int> >{};
>  bool tmp = {false};
>  (((tmp = (res.push_back(__args0) , false)) = (res.push_back(__args1) , false)) = (res.push_back(__args2) , false)) = (res.push_back(__args3) , false);
>  return std::vector<int, std::allocator<int> >(static_cast<std::vector<int, std::allocator<int> > &&>(res));
>}
>int main()
>{
>  std::vector<int, std::allocator<int> > v = Reverse(1, 2, 3, 4);
>  return 0;
>}
>```
可以看到，这里对于折叠表达式的展开和刚刚的结论是一致的。  
