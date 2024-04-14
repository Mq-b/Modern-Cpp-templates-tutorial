# 使用模板包装C风格API进行调用

这可以说是一个非常经典的需求，并且它涉及到的模板知识并不多，主要其实也就是可变参数，元组的处理，更多的还是一个思路写法，经典的 `void*` + 变参模板。

我们的写法完全参照 [MSVC STL](https://github.com/microsoft/STL) 实现的 [**`std::thread`**](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread)。在阅读本节内容之前，希望各位已经学习了现代 C++ 并发编程教程中，[**`std::thread` 的构造-源码解析**](https://github.com/Mq-b/ModernCpp-ConcurrentProgramming-Tutorial/blob/main/md/%E8%AF%A6%E7%BB%86%E5%88%86%E6%9E%90/01thread%E7%9A%84%E6%9E%84%E9%80%A0%E4%B8%8E%E6%BA%90%E7%A0%81%E8%A7%A3%E6%9E%90.md)。因为 std::thread 构造函数实际就是将我们传入的所有可调用对象、参数，包装为函数指针，和 `void*` 参数，调用 win32 的创建线程的函数 [**_beginthreadex**](https://learn.microsoft.com/zh-cn/cpp/c-runtime-library/reference/beginthread-beginthreadex?view=msvc-170)。

简单来说，我们需要写一个类包装一个这样的函数 `f` ，支持任意可调用对象与任意类型和个数的参数，最终都执行函数 `f`。

```cpp
void f(unsigned(*start_address)(void*),void *args){
    start_address(args);
    std::cout << "f\n";
}
```

它和创建线程的函数很像，一个函数指针是要执行的函数，一个 `void*` 是参数。

---

## 答案与解释

答案如下：

```cpp
struct X {
    template<typename Fn, typename ...Args>
    X(Fn&& func, Args&&... args) {
        using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;
        auto Decay_copied = std::make_unique<Tuple>(std::forward<Fn>(func), std::forward<Args>(args)...);
        auto Invoker_proc = start<Tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
        f(Invoker_proc, Decay_copied.release());
    }
    template <typename Tuple, std::size_t... Indices>
    static constexpr auto start(std::index_sequence<Indices...>) noexcept {
        return &Invoke<Tuple, Indices...>;
    }

    template <class Tuple, std::size_t... Indices>
    static unsigned int Invoke(void* RawVals) noexcept {
        const std::unique_ptr<Tuple> FnVals(static_cast<Tuple*>(RawVals));
        Tuple& Tup = *FnVals.get();
        std::invoke(std::move(std::get<Indices>(Tup))...);
        return 0;
    }
};
```

> [测试结果](https://godbolt.org/z/xePPc8aoM)。

其实很简单，就三个函数而已。**这里的难点只是将我们的可调用对象转换为 `unsigned(*start_address)(void*)` 这样类型的函数指针以及处理可变参数罢了**。我们的做法也很简单，利用模板，做了一个代码生成，实际我们传递的是静态成员函数模板 `Invoke` 给函数 `f` 调用，当然，是实例化之后的，还用到了 `start` 函数。传递的所有参数则**使用了一个元组存储副本**，由独占的智能指针管理。最终都传递给函数 `f` 调用。

好，接下来我们来一句一句解析：

1. `using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;` 定义了一个元组别名，其元组的模板类型参数就是传递给构造函数的**所有对象的类型**。

2. `auto Decay_copied = std::make_unique<Tuple>(std::forward<Fn>(func), std::forward<Args>(args)...);`定义一个独占智能指针，指向了一个元组，其存储了传递给构造函数的**所有的参数的副本**。

3. `auto Invoker_proc = start<Tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});` 调用静态成员函数模板 `start` 得到一个普通函数指针。这里需要详细展开。传递了类型参数 `Tuple` ，已经使用 `std::make_index_sequence` 制造了一个可变参数序列，用来遍历元组。

   > **[`std::index_sequence`](https://en.cppreference.com/w/cpp/utility/integer_sequence) 和 `std::make_index_sequence` 的用法我们用一个简单[示例](https://godbolt.org/z/dv88aPGac)介绍一下。**

   ```cpp
   template <typename Tuple, std::size_t... Indices>
   static constexpr auto start(std::index_sequence<Indices...>) noexcept {
       return &Invoke<Tuple, Indices...>;
   }
   ```

   将模板参数转发给 `Invoke` 进行实例化，获取这个静态成员函数模板的地址，也就是普通符合类型要求的函数指针了。

   ```cpp
   template <class Tuple, std::size_t... Indices>
   static unsigned int Invoke(void* RawVals) noexcept {
       const std::unique_ptr<Tuple> FnVals(static_cast<Tuple*>(RawVals));
       Tuple& Tup = *FnVals.get();
       std::invoke(std::move(std::get<Indices>(Tup))...);
       return 0;
   }
   ```

   我们的 `Invoke` 利用模板生成代码，支持了所有可调用类型，以及遍历元组的参数。**无非是把 `void*` 指针转换为正确的类型再去使用罢了，而这个“正确的类型”，通过模板传递。**最终的调用，在 `std::invoke(std::move(std::get<Indices>(Tup))...);` 这一行，如你所见，默认移动，和 `std::thread` 一样。

4. `f(Invoker_proc, Decay_copied.release());` 将函数指针 `Invoker_proc` 和存储了传递给构造函数的所有参数的副本的智能指针 `Decay_copied` 释放所有权，返回原始指针，用来调用 C 风格函数 f。

---

## 使用示例

```cpp
void func(int& a){
    std::cout << &a << '\n';
}

int main(){
    int a{};
    std::cout << &a << '\n';
    X{ func,a };
}
```

> [测试代码](https://godbolt.org/z/aT78bbWaz)。

和 `std::thread` 一样，上面代码无法通过编译，”*`invoke` 未找到匹配的重载函数*“。原因很简单，我们上面的代码展示了，最终的 `invoke` 调用是用了 `std::move` 的，参数被转换为右值表达式，形参类型是左值引用，左值引用不能引用右值表达式自然不行了。

```cpp
void func(const int& a){
    std::cout << &a << '\n';
}

int main(){
    int a{};
    std::cout << &a << '\n';
    X{ func,a };
}
```

> [测试代码](https://godbolt.org/z/qTPofrW6z)。

我们还可以将 `func` 的形参类型改为 `const int&` ，这可以通过编译，因为 `const int&` 可以引用右值表达式。当然了，打印的**地址不同**。原因也很简单，我们说了，智能指针**存储的是参数副本**，元组类型是使用 [`std::decay_t`](https://zh.cppreference.com/w/cpp/types/decay) 删除了 CV 与引用限定的。

```cpp
void func(const int& a){
    std::cout << &a << '\n';
}

int main(){
    int a{};
    std::cout << &a << '\n';
    X{ func,std::ref(a) };
}
```

> [测试代码](https://godbolt.org/z/9srsc8dfT)。

同样的，和 `std::thread` 一样使用 `std::ref` 即可解决。

## 总结

如果感受到难度，重新细读[**`std::thread` 的构造-源码解析**](https://github.com/Mq-b/ModernCpp-ConcurrentProgramming-Tutorial/blob/main/md/%E8%AF%A6%E7%BB%86%E5%88%86%E6%9E%90/01thread%E7%9A%84%E6%9E%84%E9%80%A0%E4%B8%8E%E6%BA%90%E7%A0%81%E8%A7%A3%E6%9E%90.md) ，或者再把[**第一部分-基础初始**](../第一部分-基础知识/)知识好好学习。

如果您还是不能全部理解，那也不用过度担心，请收藏，需要的时候照着使用，写多了很多时候往往突然就能懂了，**关键在于应用**。
