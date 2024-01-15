# SFINAE

“代换失败不是错误” (Substitution Failure Is Not An Error)

在**函数模板的重载决议**[^1]中会应用此规则：当模板形参在替换成显式指定的类型或推导出的类型失败时，从重载集中丢弃这个特化，*而非导致编译失败*。

此特性被用于模板元编程。

# 解释

对函数模板形参进行两次代换（由模板实参所替代）：

- 在模板实参推导前，对显式指定的模板实参进行代换

- 在模板实参推导后，对推导出的实参和从默认项获得的实参进行替换

代换的实参写出时非良构[^2]（并带有必要的诊断）的任何场合，都是*代换失败*。



## 代换失败与硬错误

> **只有在函数类型或其模板形参类型或其 explicit 说明符 (C++20 起)的立即语境中的类型与表达式中的失败，才是 *SFINAE 错误*。如果对代换后的类型/表达式的求值导致副作用，例如实例化某模板特化、生成某隐式定义的成员函数等，那么这些副作用中的错误都被当做*硬错误***。

> 代换失败就是指 SFINAE 错误。

以上概念中注意关键词“SFINAE 错误”、“硬错误”，这些解释不用在意，先看完以下示例再去看概念理解。

```cpp
#include <iostream>

template<typename A>
struct B { using type = typename A::type; }; // 待决名，C++20 之前必须使用 typename 消除歧义

template<
    class T,
    class U = typename T::type,              // 如果 T 没有成员 type 那么就是 SFINAE 失败（代换失败）
    class V = typename B<T>::type>           // 如果 T 没有成员 type 那么就是硬错误 不过标准保证这里不会发生硬错误，因为到 U 的默认模板实参中的代换会首先失败
void foo(int) { std::puts("SFINAE T::type B<T>::type"); }

template<typename T>
void foo(double) { std::puts("SFINAE T"); }

int main(){
    struct C { using type = int; };

    foo<B<C>>(1);       // void foo(int)    输出: SFINAE T::type B<T>::type
    foo<void>(1);       // void foo(double) 输出: SFINAE T
}
```

全平台[测试通过](https://godbolt.org/z/88bPesedP)。

以上的示例很好的向我们展示了 SFINAE 的作用，可以影响重载决议。

`foo<B<C>>(1)`、` foo<void>(1)` 如果根据一般直觉，他们都会选择到 `void foo(int)`，然而实际却不是如此；

这是因为 `foo<void>(1);` 去尝试匹配 `void foo(int)` 的时候，模板实参类型 `void` 进行替换，就会变成：

```cpp
template<
    class void,
    class U = typename void::type,         // SFINAE 失败
    class V = typename B<void>::type>      // 不会发生硬错误，因为 U 的代换已经失败
```

**`void::type`** 这一看就是非良构[^2]，根据前面提到的：

> 代换的实参写出时非良构（并带有必要的诊断）的任何场合，都是代换失败。

所以这是一个代换失败，但是因为“*代换失败不是错误*”，只是从“*重载集中丢弃这个特化，而不会导致编译失败*”，然后就就去尝试匹配 `void foo(double)` 了，`1` 是 int 类型，隐式转换到 double，没什么问题。

至于其中提到的*硬错误*？为啥它是硬错误？其实最开始的概念已经说了：

> 如果对代换后的类型/表达式的求值导致副作用，例如实例化某模板特化、生成某隐式定义的成员函数等，那么这些副作用中的错误都被当做硬错误。

`B<T>::type` 显然是对代换后的类型求值导致了副作用，实例化了模板，自然被当做硬错误。

---

这节内容非常重要，提到的概念和代码需要全部掌握，后面的内容其实无非都是以本节为基础的变种、各种使用示例、利用标准库的设施让写法简单一点，但是根本的原理，就是本节讲的。

[^1]: 注：“[重载决议](https://zh.cppreference.com/w/cpp/language/overload_resolution)”，简单来说，一个函数被重载，编译器必须决定要调用哪个重载，我们决定调用的是各形参与各实参之间的匹配最紧密的重载。

[^2]: 注：[非良构（ill-formed）](https://zh.cppreference.com/w/cpp/language/ub)——程序拥有语法错误或可诊断的语义错误。遵从标准的 C++ 编译器必须为此给出诊断