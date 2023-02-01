#include "threadpool.h"

ThreadPool::ThreadPool(int thread_num) : workers_(thread_num) {
    for (int i = 0; i < thread_num; i++) {
        std::thread t([&]{
            std::unique_lock<std::mutex> locker(mtx_);
            while (true) {
                if (!task_queue_.empty()) {
                    std::function<void()> task = task_queue_.front();
                    task_queue_.pop_front();
                    locker.unlock();
                    task();
                    locker.lock();
                } else if (stop_) {
                    break;
                } else {
                    cond_.wait(locker);
                }
            }
        });
        workers_[i] = std::move(t);
    }
}

ThreadPool::~ThreadPool() {
    stop_ = true;
    cond_.notify_all();
    for (auto& t : workers_) {
        t.join();
    }
}

template<typename Func>
void ThreadPool::AddTask(Func&& func) {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        if (stop_) return;
        task_queue_.emplace_back(std::forward<Func>(func));
    }
    cond_.notify_one();
}

