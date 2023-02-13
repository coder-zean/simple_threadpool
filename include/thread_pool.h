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
#include <tuple>
#include <vector>

#include "function_wrapper.h"

class ThreadPool {
public:
  using TaskType = std::function<void()>;

  static const uint DEFAULT_THREAD_NUM = 8;

  static uint GetDefaultThreadNum() { return DEFAULT_THREAD_NUM; }

  explicit ThreadPool(int thread_num = std::thread::hardware_concurrency());
  ThreadPool() = delete;
  ~ThreadPool();

  // AddTask方法用于不用获取函数返回值的场景
  template <typename Callable, typename... Args>
  void AddTask(Callable&& callable, Args&&... args) {
    // lambda在出了作用域之后调用，可能会存在引用失效问题，所以这里不能用引用传递，而是使用值传递
    auto task = [callable, args...] { MakeFuncWrapper(callable, args...)(); };
    {
      std::lock_guard<std::mutex> locker(mtx_);
      task_list_.emplace(task);
    }
    cond_.notify_one();
  }

  // Submit方法用于需要获取函数返回值的场景
  template <typename Callable, typename... Args>
  auto Submit(Callable&& callable, Args&&... args)
      -> std::future<typename std::remove_reference<
          decltype(MakeFuncWrapper(callable, args...)())>::type> {
    using ReturnType = typename std::remove_reference<decltype(MakeFuncWrapper(
        callable, args...)())>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        MakeFuncWrapper(callable, args...));
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