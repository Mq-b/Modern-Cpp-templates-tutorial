// example 1 
// Qt批量锁定/释放UI控件
template<typename... Args>
void lock(Args... w)
{
	(w->setDisabled(true), ...);
}

template<typename... Args>
void unlock(Args... w)
{
	(w->setDisabled(false), ...);
}

// example 2
// 函数式生成文件(夹)路径
template<typename... Args>
std::string make_relative_path(Args&&... args)
{
	std::string path{ '.' };
	(path.append('/' + std::forward<decltype(args)>(args)), ...);
	return path;
}


// example 3
// 打包成员函数任务
template <typename T>
struct is_member_function_of;

template <typename M, typename T>
struct is_member_function_of<M T::*>
{
	using type = T;
};

template<typename F, typename... Args>
auto packaged(F member_function_pointer, Args&&...args) {
	std::packaged_task packaged_task([this, member_function_pointer, ...args = std::forward<Args>(args)]() {
		using instance_t = is_member_function_of<F>::type;
		instance_t* instance = dynamic_cast<instance_t*>(this);
		return (instance->*member_function_pointer)(args...);
		});

	auto future = packaged_task.get_future();
	task_queue.push_back(std::move(packaged_task));

	return future;
}