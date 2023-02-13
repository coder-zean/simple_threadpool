# simple_threadpool

C++14实现的简易线程池。
- 通过AddTask方法添加任务，并忽略任务返回值。
- 通过Submit方法添加任务，可以通过其返回的future对象获取任务的结果

# 项目运行
```
$ git clone https://github.com/coder-zean/simple_threadpool.git
$ cd simple_threadpool
$ make build
$ make
```

# FINISHED
- [x] 支持所有只有按值传递参数的可调用对象任务   完成时间：2023/02/02
- [x] 支持获取任务的返回值                     完成时间：2023/02/03
- [x] 改用模板编程，支持了所有可调用类型，在函数参数中带有左值引用时，需要用std::ref包装     完成时间：2023/02/13
- [x] 支持通过AddWorkers方法增加线程池中工作线程数量  完成时间：2023/02/13


# 坑
## 面对函数重载情况，无法正确解析
因为重载函数在作为模板参数时，还没有发生重载函数解析。类型为：`<unresolved overloaded function type>`

重载解析只会发生在下面的情况中：

1. 对象或引用声明中的初始值设定项
2. 在复制表达式的右侧
3. 作为函数调用参数
4. 作为用户定义的运算符参数
5. 返回语句
6. 显式强制转换或static_cast
7. 非类型模板参数

## 面对函数参数中带有右值引用情况，无法正确运行（使用bind包装函数时）
因为bind函数中默认按值传递参数给绑定的函数，所以当函数参数中存在右值引用时，会出现将左值传给右值的错误情况。

可以将std::bind函数简单看为：
```c++
// C++14, you'll have to write a lot of boilerplate code for C++11
template <typename FuncT, typename ArgT>
auto
bind(FuncT&& func, ArgT&& arg)
{
  return
    [
      f = std::forward<FuncT>(func),
      a = std::forward<ArgT>(arg)
    ]() mutable { return f(a); };  // Notice: a is an lvalue here. It will error when f need a rvalue-reference.
}
```

## lambda表达式用引用捕获局部参数时，出了作用域延迟调用会出现引用失效问题
项目中一开始使用`[&] { callable(args...); }`的方式对传入的可调用对象和参数进行包装，然后再将其放入到任务队列中，再由其他线程来调用。其中`callable`和`args...`都是函数参数，是局部变量，作用域仅在函数中。所以出了函数作用域到线程函数调用它时，局部变量已经被销毁，再调用可能会导致崩溃。

现在改用值传递捕获：`[callable, args...] { callable(args...); }` 



