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

template <typename FuncType>
std::future<typename std::result_of<FuncType()>::type> ThreadPool::Submit(FuncType f) {
    using ResType = typename std::result_of<FuncType()>::type;
    std::packaged_task<ResType> task(std::move(f));
    std::future<ResType> res(task.get_future());
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);
        pool_->tasks.emplace(std::move(task));
    }
    pool_->cond.notify_one();
    return res;
}