/**
 * @file func_wrapper.h
 * @author zean (13071517766@163.com)
 * @brief 可调用对象包装类
 * @version 0.1
 * @date 2023-02-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <memory>

class FuncWrapper {
public:
    template<typename F>
    FuncWrapper(F&& f) : func_(new ImplType(std::move(f))) {}
    FuncWrapper(FuncWrapper&& other) : func_(std::move(other.func_)) {}
    FuncWrapper() = default;
    FuncWrapper(const FuncWrapper&) = delete;
    FuncWrapper(FuncWrapper&) = delete;

    void operator()() { func_->call(); }
    FuncWrapper& operator=(const FuncWrapper&) = delete;
    FuncWrapper& operator=(FuncWrapper&& other) {
        func_ = std::move(other.func_);
        return *this;
    }

private:
    struct ImplBase {
        virtual void call() = 0;
        virtual ~ImplBase() = default;
    };

    template<typename F>
    struct ImplType : public ImplBase {
        F func;
        ImplType(F&& f) : func(std::move(f)) {}
        void call() { func(); }
    };

    std::unique_ptr<ImplBase> func_;
};