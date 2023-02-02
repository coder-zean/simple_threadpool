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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>
#include <functional>
#include <future>

class ThreadPool {
public:
    explicit ThreadPool(int thread_num = std::thread::hardware_concurrency());
    ThreadPool() = default;
    ~ThreadPool();

    template<typename Func>
    void AddTask(Func&& func);

    // template <typename FuncType>
    // std::future<typename std::result_of<FuncType()>::type> Submit(FuncType f);
private:
    std::vector<std::thread> workers_;
    std::mutex mtx_;
    std::condition_variable cond_;
    std::deque<std::function<void()>> task_queue_;
    bool stop_ = false;
};