# 使用模板包装C风格API进行调用

这可以说是一个非常经典的需求，并且它涉及到的模板知识并不多，主要其实也就是可变参数，元组的处理，更多的还是一个思路写法，经典的 `void*` + 变参模板。

我们的写法完全参照 [MSVC STL](https://github.com/microsoft/STL) 实现的 [`std::thread`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread)。在阅读本节内容之前，希望各位已经学习了现代C++并发编程教程中，[**`std::thread` 的构造-源码解析**](https://github.com/Mq-b/ModernCpp-ConcurrentProgramming-Tutorial/blob/main/md/%E8%AF%A6%E7%BB%86%E5%88%86%E6%9E%90/01thread%E7%9A%84%E6%9E%84%E9%80%A0%E4%B8%8E%E6%BA%90%E7%A0%81%E8%A7%A3%E6%9E%90.md)。因为 std::thread 构造函数实际就是将我们传入的所有可调用对象、参数，包装为函数指针，和 `void*` 参数，调用 win32 的创建线程的函数 [_beginthreadex](https://learn.microsoft.com/zh-cn/cpp/c-runtime-library/reference/beginthread-beginthreadex?view=msvc-170)。

简单来说，我们需要写一个类包装一个这样的函数 `f` ，支持任意可调用对象与参数，最终都执行函数 `f`。

```cpp
void f(unsigned(*start_address)(void*),void *args){
    start_address(args);
    std::cout << "f\n";
}
```

它和创建线程的函数很像，一个函数指针是要执行的函数，一个 `void*` 是参数。

---

答案如下：

```cpp
struct X{
    template<typename Fn, typename ...Args>
    X(Fn&& func, Args&&... args){
        using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;
        auto Decay_copied = std::make_unique<Tuple>(std::forward<Fn>(func), std::forward<Args>(args)...);
        auto Invoker_proc = start<Tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
        f(Invoker_proc,Decay_copied.release());
    }
    template <typename Tuple, std::size_t... Indices>
    static constexpr auto start(std::index_sequence<Indices...>) noexcept {
        return &Invoke<Tuple,Indices...>;
    }

    template <class Tuple, std::size_t... Indices>
    static unsigned int Invoke(void* RawVals) noexcept /* terminates */ {
        const std::unique_ptr<Tuple> FnVals(static_cast<Tuple*>(RawVals));
        Tuple& Tup = *FnVals.get();
        std::invoke(std::move(std::get<Indices>(Tup))...);
        return 0;
    }
};
```

> [测试结果](https://godbolt.org/z/KT98hTaTE)。