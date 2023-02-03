#include "thread_pool.h"

ThreadPool::ThreadPool(int thread_num) : workers_(thread_num), stop_(false) {
  for (int i = 0; i < thread_num; i++) {
    std::thread t([&] {
      std::unique_lock<std::mutex> locker(mtx_);
      while (true) {
        if (!task_list_.empty()) {
          std::function<void()> task = task_list_.front();
          task_list_.pop();
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