/**
 * @file thread_pool.h
 * @author zean (13071517766@163.com)
 * @brief 线程池类
 * @version 0.1
 * @date 2023-02-01
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  using TaskType = std::function<void()>;

  static const uint DEFAULT_THREAD_NUM = 8;

  static uint GetDefaultThreadNum() { return DEFAULT_THREAD_NUM; }

  explicit ThreadPool(int thread_num = std::thread::hardware_concurrency());
  ThreadPool() = delete;
  ~ThreadPool();

  // 支持类成员函数作为任务添加入线程池
  template <typename TClass, typename TReturn, typename... TParam>
  void AddTask(TClass* t_this, TReturn (TClass::*t_func)(TParam...),
               TParam&&... t_param) {
    {
      std::lock_guard<std::mutex> locker(mtx_);
      auto task = std::bind(std::forward<decltype(t_func)>(t_func), t_this,
                            std::ref(t_param)...);
      task_list_.emplace([task]() { task(); });
    }
    cond_.notify_one();
  }

  // 支持无参的可调用对象作为任务添加入线程池
  template <typename Callable>
  void AddTask(Callable&& func) {
    {
      std::lock_guard<std::mutex> locker(mtx_);
      task_list_.emplace(std::forward<Callable>(func));
    }
    cond_.notify_one();
  }

  // 支持有参的可调用对象作为任务添加入线程池
  template <typename Callable, typename... TParam>
  void AddTask(Callable&& func, TParam&&... t_param) {
    {
      std::lock_guard<std::mutex> locker(mtx_);
      auto task = std::bind(std::forward<Callable>(func),
                            std::forward<TParam>(t_param)...);
      task_list_.emplace([task]() { task(); });
    }
    cond_.notify_one();
  }

  // 支持类成员函数作为任务添加入线程池
  template <typename TClass, typename TReturn, typename... TParam>
  std::future<TReturn> Submit(TClass* t_this,
                              TReturn (TClass::*t_func)(TParam...),
                              TParam&&... t_param) {
    auto task = std::make_shared<std::packaged_task<TReturn()>>(std::bind(
        std::forward<decltype(t_func)>(t_func), t_this, std::ref(t_param)...));
    std::future<TReturn> result = task->get_future();
    {
      std::lock_guard<std::mutex> locker(mtx_);
      task_list_.emplace([task]() { (*task)(); });
    }
    cond_.notify_one();
    return result;
  }

  // 支持无参的可调用对象作为任务添加入线程池
  template <typename Callable>
  auto Submit(Callable&& func) -> std::future<decltype(func())> {
    using ReturnType = decltype(func());
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Callable>(func)));
    std::future<ReturnType> result = task->get_future();
    {
      std::lock_guard<std::mutex> locker(mtx_);
      task_list_.emplace([task]() { (*task)(); });
    }
    cond_.notify_one();
    return result;
  }

  // 支持有参的可调用对象作为任务添加入线程池
  template <typename Callable, typename... TParam>
  auto Submit(Callable&& func, TParam&&... t_param)
      -> std::future<decltype(func(t_param...))> {
    using ReturnType = decltype(func(t_param...));
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(
        std::forward<Callable>(func), std::forward<TParam>(t_param)...));
    std::future<ReturnType> result = task->get_future();
    {
      std::lock_guard<std::mutex> locker(mtx_);
      task_list_.emplace([task]() { (*task)(); });
    }
    cond_.notify_one();
    return result;
  }

private:
  std::vector<std::thread> workers_;
  std::mutex mtx_;
  std::condition_variable cond_;
  std::queue<TaskType> task_list_;
  std::atomic<bool> stop_;
};