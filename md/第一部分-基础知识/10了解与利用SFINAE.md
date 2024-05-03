# SFINAE

“代换失败不是错误” (Substitution Failure Is Not An Error)

在**函数模板的重载决议**[^1]中会应用此规则：当模板形参在替换成显式指定的类型或推导出的类型失败时，从重载集中丢弃这个特化，*而非导致编译失败*。

此特性被用于模板元编程。

> 注意：**本节非常非常的重要，是模板基础中的基础，最为基本的特性和概念**。

## 解释

对函数模板形参进行两次代换（由模板实参所替代）：

- 在模板实参推导前，对显式指定的模板实参进行代换

- 在模板实参推导后，对推导出的实参和从默认项获得的实参进行替换

代换的实参写出时非良构[^2]（并带有必要的诊断）的任何场合，都是*代换失败*。

> ”对显式指定的模板实参进行代换“这里的显式指定，就比如 `f<int>()` 就是显式指明了。我知道你肯定有疑问：我都显式指明了，那下面还推导啥？对，如果模板函数 `f` 只有一个模板形参，而你显式指明了，的确第二次代换没用，因为根本没啥好推导的。

> 两次代换都有作用，是在于有多个模板形参，显式指定一些，又根据传入参数推导一些。

## 代换失败与硬错误

> **只有在函数类型或其模板形参类型或其 explicit 说明符 (C++20 起)的*立即语境*中的类型与表达式中的失败，才是 *SFINAE 错误*。如果对代换后的类型/表达式的*求值导致副作用*，例如实例化某模板特化、生成某隐式定义的成员函数等，那么这些副作用中的错误都被当做*硬错误***。

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

`foo<B<C>>(1)`、`foo<void>(1)` 如果根据一般直觉，它们都会选择到 `void foo(int)`，然而实际却不是如此；

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

`B<T>` 显然是对代换后的类型求值导致了副作用，实例化了模板，实例化失败自然被当做硬错误。

> 注意，你应当关注 `B<T>` 而非 `B<T>::type`，因为是直接在实例化模板 B 的时候就失败了，被当成硬错误；如果 `B<T>` 实例化成功，而没有 `::type`，则被当成**代换失败**（不过这里是不可能）。

---

这节内容非常重要，提到的概念和代码需要全部掌握，后面的内容其实无非都是以本节为基础的变种、各种使用示例、利用标准库的设施让写法简单一点，但是根本的原理，就是本节讲的。

## 基础使用示例

*请在完全读懂上一节内容再阅读本节内容*。

C++ 的模板，很多时候就像拼图一样，我们带入进去想，很多问题即使没有阅读规则，也可以无师自通，猜出来。

---

> ***我需要写一个函数模板 `add`，想要要求传入的对象必须是支持 `operator+` 的，应该怎么写？***

利用 SFINAE 我们能轻松完成这个需求。

```cpp
template<typename T>
auto add(const T& t1, const T& t2) -> decltype(t1 + t2){   // C++11 后置返回类型，在返回类型中运用 SFINAE
    std::puts("SFINAE +");
    return t1 + t2;
}
```

我知道你一定会有疑问

> 1. 这样有啥好处吗？使用了 SFINAE 看起来还变复杂了。我就算不用这写法，如果对象没有 `operator+` 不是一样会编译错误吗？
> 2. 虽然前面说了 SFINAE 可以影响重载决议，我知道这个很有用，但是我这个函数根本没有别的重载，这样写还是有必要的吗？

这两个问题其实是一个问题，本质上就是还是不够懂 SFINAE 或者说模板：

- 如果就是简单写一个 `add` 函数模板不使用 SFINAE，那么编译器在编译的时候，会尝试模板实例化，生成函数定义，发现你这类型根本没有 `operator+`，于是实例化模板错误。

- 如果按照我们上面的写法使用 SFINAE，根据“*代换失败不是错误*”的规则，从重载集中丢弃这个特化 `add`，然而又没有其他的 `add` 重载，所以这里的错误是“**未找到匹配的重载函数**”。

这里的重点是什么？**是模板实例化，能不要实例化就不要实例化**，我们当前的示例只是因为 `add` 函数模板非常的简单，即使实例化错误，编译器依然可以很轻松的报错告诉你，是因为没有 `operator+`。但是很多模板是非常复杂的，编译器实例化模板经常会产生一些完全不可读的报错；如果我们使用 SFINAE，编译器就是直接告诉我：“未找到匹配的重载函数”，我们自然知道就是传入的参数没有满足要求。而且实例化模板也是有开销的，很多时候甚至很大。

总而言之：
**即使不为了处理重载，使用 SFINAE 约束函数模板的传入类型，也是有很大好处的：报错、编译速度**。

但是令人诟病的是 SFINAE 的写法在很多时候非常麻烦，目前各位可能还是没有感觉，后面的需求，写出的示例，慢慢的你就会感觉到了。这些问题会在下一章的[约束与概念](/md/第一部分-基础知识/11约束与概念.md)解决。

## 标准库支持

标准库提供了一些设施帮助我们更好的使用 SFINAE。

### `std::enable_if`

```cpp
template<bool B, class T = void>
struct enable_if {};
 
template<class T> // 类模板偏特化
struct enable_if<true, T> { typedef T type; };     // 只有 B 为 true，才有 type，即 ::type 才合法

template< bool B, class T = void >
using enable_if_t = typename enable_if<B,T>::type; // C++14 引入
```

这是一个模板类，在 C++11 引入，它的用法很简单，就是第一个模板参数为 true，此模板类就有 `type`，不然就没有，以此进行 SFINAE。

```cpp
template<typename T,typename SFINAE = 
    std::enable_if_t<std::is_same_v<T,int>>>
void f(T){}
```

函数 `f` 要求 `T` 类型必须是 `int` 类型；我们一步一步分析

`std::enable_if_t<std::is_same_v<T,int>>>` 如果 T 不是 int，那么 [std::is_same_v](https://zh.cppreference.com/w/cpp/types/is_same) 就会返回一个 false，也就是说 `std::enable_if_t<false>` ，带入：

```cpp
using enable_if_t = typename enable_if<false,void>::type; // void 是默认模板实参
```

但是问题在于：

- **enable_if 如果第一个模板参数为 `false`，它根本没有 `type` 成员**。

所以这里是个**代换失败**，但是因为“*代换失败不是错误*”，所以只是不选择函数模板 `f`，而不会导致编译错误。

---

再谈，std::enable_if 的默认模板实参是 **`void`**，如果我们不在乎 std::enable_if 得到的类型，就让它默认就行，比如我们的示例 `f` 根本不在乎第二个模板形参 `SFINAE` 是啥类型。

```cpp
template <class Type, class... Args>
array(Type, Args...) -> array<std::enable_if_t<(std::is_same_v<Type, Args> && ...), Type>, sizeof...(Args) + 1>;
```

以上示例，是显式指明了 std::enable_if 的第二个模板实参，为 `Type`。

它是我们[类模板](02类模板.md)推导指引那一节的示例的**改进版本**，我们使用 std::enable_if_t 与 C++17 折叠表达式，为它增加了约束，这几乎和 [libstdc++](https://github.com/gcc-mirror/gcc/blob/7a01cc711f33530436712a5bfd18f8457a68ea1f/libstdc%2B%2B-v3/include/std/array#L292-L295) 中的代码一样。

`(std::is_same_v<Type, Args> && ...)` 做 std::enable_if 的第一个模板实参，这里是一个一元右折叠，使用了 **`&&`** 运算符，也就是必须 std::is_same_v 全部为 true，才会是 true。简单的说就是要求类型形参包 Args 中的每一个类型全部都是一样的，不然就是替换失败。

这样做有很多好处，老式写法存在很多问题：

```cpp
template<class Ty, std::size_t size>
struct array {
    Ty arr[size];
};

template<typename T, typename ...Args>
array(T t, Args...) -> array<T, sizeof...(Args) + 1>;

::array arr{1.4, 2, 3, 4, 5};        // 被推导为 array<double,5>
::array arr2{1, 2.3, 3.4, 4.5, 5.6}; // 被推导为 array<int,5>    有数据截断
```

如果不使用 SFINAE 约束，那么 array 的类型完全取决于第一个参数的类型，很容易导致其他问题。

### `std::void_t`

```cpp
template< class... >
using void_t = void;
```

如你所见，它的实现非常非常的简单，就是一个别名，接受任意个数的类型参数，但自身始终是 `void` 类型。

- 将任意类型的序列映射到类型 void 的工具元函数。

- 模板元编程中，用此元函数检测 SFINAE 语境中的非良构类型[^2]。

---

> *我要写一个函数模板 `add`，我要求传入的对象需要支持 `+` 以及它需要有别名 `type` ，成员 `value`、`f`*。

```cpp
#include <iostream>
#include <type_traits>

template<typename T,
    typename SFINAE = std::void_t<
    decltype(T{} + T{}), typename T::type, decltype(&T::value), decltype(&T::f) >>
auto add(const T& t1, const T& t2) {
    std::puts("SFINAE + | typename T::type | T::value");
    return t1 + t2;
}

struct Test {
    int operator+(const Test& t)const {
        return this->value + t.value;
    }
    void f()const{}
    using type = void;
    int value;
};

int main() {
    Test t{ 1 }, t2{ 2 };
    add(t, t2);  // OK
    //add(1, 2); // 未找到匹配的重载函数
}
```

- `decltype(T{} + T{})` 用 decltype 套起来只是为了获得类型符合语法罢了，std::void_t 只接受类型参数。如果类型没有 `operator+`，自然是*代换失败*。

- `typename T::type` 使用 `typename` 是因为[待决名](09待决名.md)；type 本身是类型，不需要 decltype。如果 `add` 推导的类型没有 `type` 别名，自然是*代换失败*。

- `decltype(&T::value)` 用 decltype 套就不用说了，`&T::value` 是[成员指针](https://zh.cppreference.com/w/cpp/language/pointer#.E6.88.90.E5.91.98.E6.8C.87.E9.92.88)的语法，不区分是数据成员还是成员函数，如果有这个成员 `value`，`&类名::成员名字` 自然合法，要是没有，就是*代换失败*。

- `decltype(&T::f)` ，其实前面已经说了，成员函数是没区别的，没有成员 `f` 就是 *代换失败*。

总而言之，这是为了使用 SFINAE。

> 那么这里 `std::void_t` 的作用是？

其实倒也没啥，无非就是给了个好的语境，让我们能这样写，最终 `typename SFINAE = std::void_t` 这里的 `SFINAE` 的类型就是 `void`；当然了，这不重要，重要的是创造这样写的语境，能够方便我们进行 **`SFINAE`**。

仅此一个示例，我相信就足够展示 `std::void_t` 的使用了。

> *那么如果在 C++17 标准之前，没有 std::void_t ，我该如何要求类型有某些成员呢？*

其实形式和原理都是一样的。

```cpp
template<typename T,typename SFINAE = decltype(&T::f)>
void f(T){}

struct Test {
    void f()const{}
};

Test t;
f(t);  // OK
f(1);  // 未找到匹配的重载函数
```

C++11 可用。

### `std::declval`

```cpp
template<class T>
typename std::add_rvalue_reference<T>::type declval() noexcept;
```

将任意类型 T 转换成引用类型，*使得在 decltype 说明符的操作数中不必经过构造函数就能使用成员函数*。

- [std::declval](https://zh.cppreference.com/w/cpp/utility/declval) 只能用于 **[不求值语境](https://zh.cppreference.com/w/cpp/language/expressions#.E6.BD.9C.E5.9C.A8.E6.B1.82.E5.80.BC.E8.A1.A8.E8.BE.BE.E5.BC.8F)**，且不要求有定义。

- **它不能被实际调用，因此不会返回值，返回类型是 `T&&`**。

它常用于模板元编程 SFINAE，我们用一个示例展现它的必要性：

```cpp
template<typename T, typename SFINAE = std::void_t<decltype(T{} + T{})> >
auto add(const T& t1, const T& t2) {
    std::puts("SFINAE +");
    return t1 + t2;
}

struct X{
    int operator+(const X&)const{
        return 0;
    }
};

struct X2 {
    X2(int){}   // 有参构造，没有默认构造函数
    int operator+(const X2&)const {
        return 0;
    }
};

int main(){
    X x1, x2;
    add(x1, x2);          // OK

    X2 x3{ 0 }, x4{ 0 };
    add(x3,x4);           // 未找到匹配的重载函数
}
```

错误的原因很简单，`decltype(T{} + T{})` 这个表达式中，同时**要求了 `T` 类型支持默认构造**（虽然这不是我们的本意），然而我们的 `X2` 类型没有默认构造，自然而然 `T{}` 不是合法表达式，*代换失败*。其实我们之前也有类似的写法，我们在本节进行纠正，使用 `std::declval`：

```cpp
template<typename T, typename SFINAE = std::void_t<decltype(std::declval<T>() + std::declval<T>())> >
auto add(const T& t1, const T& t2) {
    std::puts("SFINAE +");
    return t1 + t2;
}
```

[测试](https://godbolt.org/z/7GGWvd5PM)。

把 `T{}` 改成 `std::declval<T>()` 即可，decltype 是不求值语境，没有问题。

---

还不止如此，使用它得以让我们先前的 `SFINAE` 检查类型是否有某些成员的形式得以改进，而不是像之前一样的  `decltype(&T::value), decltype(&T::f)` 的利用成员指针的形式。

```cpp
template<typename T,typename SFINAE = decltype(std::declval<T>().f(1))>
void f(int) { std::puts("f int"); }

template<typename T, typename SFINAE = decltype(std::declval<T>().f())>
void f(double) { std::puts("f"); }

struct X{
    void f()const{}
};
struct Y{
    void f(int)const{}
};

int main(){
    f<X>(1);
    f<Y>(1.1);
}
```

[**运行结果**](https://godbolt.org/z/vvdWKKM5n)：

```txt
f
f int
```

显而易见，虽然我们的 `f<X>(1)` 传递的参数是 int 类型，但是却打印了 `f`，也就是代表实际匹配到了参数为 `f(double)` 的版本，这是因为我们的 `f(int)` 版本的 `SFINAE` 约束要求了类型必须是支持 `f(1)` 这种形式，`X` 的成员函数 `f` 是空参的，自然不满足。

**使用此种方式得以更加明确的约束，因为不管成员函数 f 的形参是什么情况，其成员指针表示形式都是：`&类名::f`。**

数据成员同样可以使用 `declval` 进行约束：

```cpp
template<typename T, typename SFINAE = decltype(std::declval<T>().value)>
void f(int) { std::puts("f value"); }

template<typename T>
void f(double) { std::puts("f"); }

struct X {
    int value{};
};
struct Y {};

int main() {
    f<X>(1); // f value
    f<Y>(1); // f
}
```

[**运行结果**](https://godbolt.org/z/c3ahK1WxP)：

```txt
f value
f
```

`f<Y>(1)`  虽然传递的参数是 int 类型，但是因为 `Y` 不满足 `SFINAE` 的约束，即没有成员 `value`，所以只能选择到 `f(double)` 的版本。

## 部分（偏）特化中的 SFINAE

在确定一个类或变量 (C++14 起)模板的特化是由部分特化还是主模板生成的时候也会出现推导与替换。在这种确定期间，**部分特化的替换失败不会被当作硬错误，而是像函数模板一样*代换失败不是错误*，只是忽略这个部分特化**。

```cpp
#include <iostream>

template<typename T,typename T2 = void>
struct X{
    static void f() { std::puts("主模板"); }
};

template<typename T>
struct X<T, std::void_t<typename T::type>>{
    using type = typename T::type;
    static void f() { std::puts("偏特化 T::type"); }
};

struct Test { using type = int; };
struct Test2 { };

int main(){
    X<Test>::f();       // 偏特化 T::type
    X<Test2>::f();      // 主模板
}
```

## 总结

到此，其实就足够了，SFINAE 的原理、使用、标准库支持（std::enable_if、std::void_t、std::declval）。

虽然称不上全部，但如果你能完全理解明白本节的所有内容，那你一定超越了至少 95% C++ 开发者。其他的各种形式无非都是这样类似的，因为我们已经为你讲清楚了 ***原理***。

- ***代换失败不是错误***。

[^1]: 注：“[重载决议](https://zh.cppreference.com/w/cpp/language/overload_resolution)”，简单来说，一个函数被重载，编译器必须决定要调用哪个重载，我们决定调用的是各形参与各实参之间的匹配最紧密的重载。

[^2]: 注：[非良构（ill-formed）](https://zh.cppreference.com/w/cpp/language/ub)——程序拥有语法错误或可诊断的语义错误。遵从标准的 C++ 编译器必须为此给出诊断。
