# CRTP 奇特重现模板模式

它的范式基本上都是：父类是模板，再定义一个类类型继承它。因为类模板不是类，只有实例化后的类模板才是实际的类类型，所以我们需要实例化它，**显式指明模板类型参数，而这个类型参数就是我们定义的类型，也就是子类了**。

```cpp
template <class Dervied>
class Base {};

class X : public Base<X> {};
```

这种范式是完全合法合理的，并无问题，首先不要对这种东西感到害怕或觉得非常神奇，也只不过是基本的语法规则所允许的。即使不使用 `CRTP` ，这些写法也是完全合理的，并无问题。

---

CRTP 可用于在父类暴露接口，而子类实现该接口，以此实现“***编译期多态***”，或称“***静态多态***”。 示例如下：

```cpp
template <class Dervied>
class Base {
public:
    // 公开的接口函数 供外部调用
    void addWater(){
        // 调用子类的实现函数。要求子类必须实现名为 impl() 的函数
        // 这个函数会被调用来执行具体的操作
        static_cast<Dervied*>(this)->impl();
    }
};

class X : public Base<X> {
public:
    // 子类实现了父类接口
    void impl() const{
        std::cout<< "X 设备加了 50 毫升水\n";
    }
};
```

使用方式也很简单，我们直接创建子类对象，调用 `addWater` 函数即可：

```cpp
X x;
x.addWater();
```

> [运行](https://godbolt.org/z/o373avza5)测试。

那么好，问题来了，**为什么呢？** `static_cast<Dervied*>(this)` 是做了什么，它为什么可以这样？

- 很显然 `static_cast<Dervied*>(this)` 是进行了一个类型转换，将 `this` 指针（也就是**父类的指针**），转换为通过模板参数传递的类型，也就是**子类的指针**。
- **这个转换是安全合法的**。因为 this 指针实际上指向一个 X 类型的对象，X 类型对象继承了 `Base<X>` 的部分，X 对象也就包含了 `Base<X>` 的部分，所以这个转换在编译期是有效的，并且是合法的。
- 当你调用 `x.addWater()` 时，实际上是 X 对象调用了父类 `Base<X>` 的成员函数。这个成员函数内部使用 `static_cast<X*>(this)`，将 this 从 `Base<X>*` 转换为 `X*`，然后调用 X 中的 impl() 函数。这种转换是合法且安全的，且 X 确实实现了 impl() 函数。

当然了，我们给出的示例是十分简单的，不过大多的使用的确也就是如此了，我们可以再优化一点，比如不让子类的接口暴露出来：

```cpp
template <class Dervied>
class Base {
public:
    void addWater(){
        static_cast<Dervied*>(this)->impl();
    }
};

class X : public Base<X> {
    // 设置友元，让父类得以访问
    friend Base<X>;
    // 私有接口，禁止外部访问
    void impl() const{
        std::cout<< "X 设备加了 50 毫升水\n";
    }
};
```

## C++23 的改动-显式对象形参

C++23 引入了**显式对象形参**，让我们的 `CRTP` 的形式也出现了变化：

> [显式对象形参](https://zh.cppreference.com/w/cpp/language/member_functions#.E6.98.BE.E5.BC.8F.E5.AF.B9.E8.B1.A1.E6.88.90.E5.91.98.E5.87.BD.E6.95.B0)，顾名思义，就是将 C++23 之前，隐式的，由编译器自动将 `this` 指针传递给成员函数使用的，改成**允许用户显式写明**了，也就是：
>
> ```cpp
> struct X{
>     void f(this const X& self){}
> };
> ```
>
> 它也支持模板（可以直接 `auto` 而无需再 `template<typename>`），也支持各种修饰，如：`this X self`、`this X& self`、`this const X& self`、`this X&& self`、`this auto&& self`、`const auto& self` ... 等等。

```cpp
struct Base { void name(this auto&& self) { self.impl(); } };
struct D1 : Base { void impl() { std::puts("D1::impl()"); } };
struct D2 : Base { void impl() { std::puts("D2::impl()"); } };
```

不再需要使用 `static_cast` 进行转换，直接调用即可。且如你所见，我们的显式对象形参也可以写成模板的形式：`this auto&& self`。

使用上也与之前并无区别，创建子类对象，调用接口即可。

```cpp
D1 d;
d.name();
D2 d2;
d2.name();
```

`d.name` 也就是把 `d` 传入给父类模板成员函数 `name`，`auto&&` 被推导为 `D1&`，顾名思义”***显式***“对象形参，非常的简单直观。

> [运行](https://godbolt.org/z/WW59PqEd3)测试。

## CRTP 的好处

上一节我们详细的介绍和解释了 CRTP 的编写范式和原理。现在我们来稍微介绍一下 CRTP 的众多好处。

1. **静态多态**

   CRTP 实现静态多态，无需使用虚函数，静态绑定，无运行时开销。
2. **类型安全**

   CRTP 提供了类型安全的多态性。通过模板参数传递具体的子类类型，编译器能够确保类型匹配，避免了传统向下转换可能引发的类型错误。
3. **灵活的接口设计**

   CRTP 允许父类定义公共接口，并要求子类实现具体的操作。这使得基类能够提供通用的接口，而具体的实现细节留给派生类。其实也就是说多态了。

## 总结

事实上笔者所见过的 `CRTP` 的用法也还不止如此，还有许多更加复杂，有趣的做法，不过就不想再探讨了，以上的内容已然足够。其它的做法也无非是基于以上了。

各位可以尝试尽可能的将使用虚函数的代码改成 `CRTP` ，这其实在大多数时候并不构成难度，绝大多数的多态类型都能被很轻松的改成 `CRTP` 的形式。
