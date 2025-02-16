# Linux 中封装 POSIX 接口编写 thread 类

在我们上一节内容其实就提到了 `std::thread` 的 msvc stl 的实现，本质上就是封装了 win32 的那些接口，包括其中最重要的就是利用了模板技术接受任意可调用类型，将其转发给 C 的只是接受单一函数指针的 `_beginthreadex` 去创建线程。

上一节中我们只是讲了单纯的讲了 “*模板包装C风格API进行调用*”，这一节我们就来实际一点，直接封装编写一个自己的 `std::thread`，使用 POSIX 接口。

我们将在 Ubuntu22.04 中使用 gcc11.4 开启 C++17 标准进行编写和测试。

## 实现

### 搭建框架

```cpp
namespace mq_b{
    class thread{
    public:
        class id;

        id get_id() const noexcept;
    };

    namespace this_thread {
        [[nodiscard]] thread::id get_id() noexcept;
    }

    class thread::id {
    public:
        id() noexcept = default;

    private:
        explicit id(pthread_t other_id) noexcept : Id(other_id) {}

        pthread_t Id;

        friend thread::id thread::get_id() const noexcept;
        friend thread::id this_thread::get_id() noexcept;
        friend bool operator==(thread::id left, thread::id right) noexcept;

        template <class Ch, class Tr>
        friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& str, thread::id Id);
    };

    [[nodiscard]] inline thread::id thread::get_id() const noexcept {
        return thread::id{ Id };
    }

    [[nodiscard]] inline thread::id this_thread::get_id() noexcept {
        return thread::id{ pthread_self() };
    }

    template <class Ch, class Tr>
    std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& str, thread::id Id){
        str << Id.Id;
        return str;
    }
}
```

我们先搭建一个基础的架子给各位， 定义我们自己的命名空间，定义内部类 `thread::id`，以及运算符重载。这些并不涉及什么模板技术，我们先进行编写，其实重点只是构造函数而已，剩下的其它成员函数也很简单，我们一步一步来。

另外，**POSIX 的线程函数都只需要一个句柄，其实就是无符号 int 类型就能操作**，别名是 `pthread_t` 所以我们只需要保有这样一个 id 就好了。

### 实现构造函数

```cpp
template<typename Fn, typename ...Args>
thread(Fn&& func, Args&&... args) {
    using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;
    auto Decay_copied = std::make_unique<Tuple>(std::forward<Fn>(func), std::forward<Args>(args)...);
    auto Invoker_proc = start<Tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
    if (int result = pthread_create(&Id, nullptr, Invoker_proc, Decay_copied.get()); result == 0) {
        (void)Decay_copied.release();
    }else{
        std::cerr << "Error creating thread: " << strerror(result) << std::endl;
        throw std::runtime_error("Error creating thread");
    }
}
template <typename Tuple, std::size_t... Indices>
static constexpr auto start(std::index_sequence<Indices...>) noexcept {
    return &Invoke<Tuple, Indices...>;
}

template <class Tuple, std::size_t... Indices>
static void* Invoke(void* RawVals) noexcept {
    const std::unique_ptr<Tuple> FnVals(static_cast<Tuple*>(RawVals));
    Tuple& Tup = *FnVals.get();
    std::invoke(std::move(std::get<Indices>(Tup))...);
    nullptr 0;
}
```

这其实很简单，几乎是直接复制了我们上一节的内容，只是把函数改成了 `pthread_create`，然后多传了两个参数，以及修改了 Invoke 的返回类型和 return，确保类型符合 `pthread_create` 。

### 完善其它成员函数

然后再稍加完善那些简单的成员函数，也就是：

```cpp
~thread(){
    if (joinable())
        std::terminate();
}

thread(const thread&) = delete;

thread& operator=(const thread&) = delete;

thread(thread&& other) noexcept : Id(std::exchange(other.Id, {})) {}

thread& operator=(thread&& t) noexcept{
    if (joinable())
        std::terminate();
    swap(t);
    return *this;
}

void swap(thread& t) noexcept{
    std::swap(Id, t.Id);
}

bool joinable() const noexcept{
    return !(Id == 0);
}

void join() {
    if (!joinable()) {
        throw std::runtime_error("Thread is not joinable");
    }
    int result = pthread_join(Id, nullptr);
    if (result != 0) {
        throw std::runtime_error("Error joining thread: " + std::string(strerror(result)));
    }
    Id = {}; // Reset thread id
}

void detach() {
    if (!joinable()) {
        throw std::runtime_error("Thread is not joinable or already detached");
    }
    int result = pthread_detach(Id);
    if (result != 0) {
        throw std::runtime_error("Error detaching thread: " + std::string(strerror(result)));
    }
    Id = {}; // Reset thread id
}

id get_id() const noexcept;

native_handle_type native_handle() const{
    return Id;
}
```

我觉得无需多言，这些都十分的简单。然后就完成了，对，就是这么简单。

### 完整代码与测试

**完整实现代码**：

```cpp
namespace mq_b{
    class thread{
    public:
        class id;
        using native_handle_type = pthread_t;

        thread() noexcept :Id{} {}

        template<typename Fn, typename ...Args>
        thread(Fn&& func, Args&&... args) {
            using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;
            auto Decay_copied = std::make_unique<Tuple>(std::forward<Fn>(func), std::forward<Args>(args)...);
            auto Invoker_proc = start<Tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
            if (int result = pthread_create(&Id, nullptr, Invoker_proc, Decay_copied.get()); result == 0) {
                (void)Decay_copied.release();
            }else{
                std::cerr << "Error creating thread: " << strerror(result) << std::endl;
                throw std::runtime_error("Error creating thread");
            }
        }
        template <typename Tuple, std::size_t... Indices>
        static constexpr auto start(std::index_sequence<Indices...>) noexcept {
            return &Invoke<Tuple, Indices...>;
        }

        template <class Tuple, std::size_t... Indices>
        static void* Invoke(void* RawVals) noexcept {
            const std::unique_ptr<Tuple> FnVals(static_cast<Tuple*>(RawVals));
            Tuple& Tup = *FnVals.get();
            std::invoke(std::move(std::get<Indices>(Tup))...);
            return nullptr;
        }

        ~thread(){
            if (joinable())
                std::terminate();
        }

        thread(const thread&) = delete;

        thread& operator=(const thread&) = delete;

        thread(thread&& other) noexcept : Id(std::exchange(other.Id, {})) {}

        thread& operator=(thread&& t) noexcept{
            if (joinable())
                std::terminate();
            swap(t);
            return *this;
        }

        void swap(thread& t) noexcept{
            std::swap(Id, t.Id);
        }

        bool joinable() const noexcept{
            return !(Id == 0);
        }

        void join() {
            if (!joinable()) {
                throw std::runtime_error("Thread is not joinable");
            }
            int result = pthread_join(Id, nullptr);
            if (result != 0) {
                throw std::runtime_error("Error joining thread: " + std::string(strerror(result)));
            }
            Id = {}; // Reset thread id
        }

        void detach() {
            if (!joinable()) {
                throw std::runtime_error("Thread is not joinable or already detached");
            }
            int result = pthread_detach(Id);
            if (result != 0) {
                throw std::runtime_error("Error detaching thread: " + std::string(strerror(result)));
            }
            Id = {}; // Reset thread id
        }

        id get_id() const noexcept;

        native_handle_type native_handle() const{
            return Id;
        }

        pthread_t Id;
    };

    namespace this_thread {
        [[nodiscard]] thread::id get_id() noexcept;
    }

    class thread::id {
    public:
        id() noexcept = default;

    private:
        explicit id(pthread_t other_id) noexcept : Id(other_id) {}

        pthread_t Id;

        friend thread::id thread::get_id() const noexcept;
        friend thread::id this_thread::get_id() noexcept;
        friend bool operator==(thread::id left, thread::id right) noexcept;

        template <class Ch, class Tr>
        friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& str, thread::id Id);
    };

    [[nodiscard]] inline thread::id thread::get_id() const noexcept {
        return thread::id{ Id };
    }

    [[nodiscard]] inline thread::id this_thread::get_id() noexcept {
        return thread::id{ pthread_self() };
    }

    template <class Ch, class Tr>
    std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& str, thread::id Id){
        str << Id.Id;
        return str;
    }
}
```

**标头**：

```cpp
#include <iostream>
#include <thread>
#include <functional>
#include <tuple>
#include <utility>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
```

**测试**：

```cpp
void func(int& a) {
    std::cout << &a << '\n';
}
void func2(const int& a){
    std::cout << &a << '\n';
}
struct X{
    void f() { std::cout << "X::f\n"; }
};

int main(){
    std::cout << "main thread id: " << mq_b::this_thread::get_id() << '\n';

    int a = 10;
    std::cout << &a << '\n';
    mq_b::thread t{ func,std::ref(a) };
    t.join();

    mq_b::thread t2{ func2,a };
    t2.join();

    mq_b::thread t3{ [] {std::cout << "thread id: " << mq_b::this_thread::get_id() << '\n'; } };
    t3.join();

    X x;
    mq_b::thread t4{ &X::f,&x };
    t4.join();

    mq_b::thread{ [] {std::cout << "👉🤣\n"; } }.detach();
    sleep(1);
}
```

> [运行](https://godbolt.org/z/1j48Mh89x)测试。

项目：<https://github.com/Mq-b/POSIX-thread>

## 总结

其实这玩意没多少难度，唯一的难度就只有那个构造函数而已，剩下的代码和成员函数，甚至可以照着标准库抄一些，或者就是调用 POSIX 接口罢了。

不过如果各位能完全理解明白，那也足以自傲，毕竟的确没多少人懂。简单是相对而言的，如果你跟着视频一直学习了前面的模板，并且有基本的并发的知识，对 `POSIX` 接口有基本的认识，以及看了前面提到的 [**《`std::thread` 的构造-源码解析》**](https://mq-b.github.io/ModernCpp-ConcurrentProgramming-Tutorial/md/%E8%AF%A6%E7%BB%86%E5%88%86%E6%9E%90/01thread%E7%9A%84%E6%9E%84%E9%80%A0%E4%B8%8E%E6%BA%90%E7%A0%81%E8%A7%A3%E6%9E%90.html)，那么本节的内容对你，肯定不构成难度。
