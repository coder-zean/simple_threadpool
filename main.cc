#include "thread_pool.h"
#include <iostream>
#include <functional>
#include <vector>

using namespace std;

void test_func(int x) {
    cout << "test(int x)" << endl;
}

int test_func_res(double y) {
    cout << "int test_func_res(double y)" << endl;
    return static_cast<int>(y);
}

int test_func_res_zero_arg() {
    cout << "int test_func_res_zero_arg()" << endl;
    return 400;
}


class TestCallable {
public:
    void operator()() {
        cout << "TestCallable::operator()()" << endl;
    }
    

    int operator()(int a) {
        cout << "int TestCallable::operator()(int a)" << endl;
        return a;
    }
};

class Test {
public:
    Test() : threadpool_(8) {}

    void Print(int& i) {
        cout << "Test::Print(int& i)" << endl;
    }

    int PrintWithRes(int i) {
        cout << "PrintWithRes(int i)" << endl;
        return i;
    }

    void Add() {
        threadpool_.AddTask(this, &Test::Print, x_);
        threadpool_.AddTask(&test_func, 10);
        TestCallable test_call;
        threadpool_.AddTask(test_call);
        std::future<int> f1 = threadpool_.Submit(this, &Test::PrintWithRes, 100);
        cout << "future res = " << f1.get() << endl;
        std::future<int> f2 = threadpool_.Submit(&test_func_res, 200.0);
        cout << "future res = " << f2.get() << endl;
        std::future<int> f3 = threadpool_.Submit(test_call, 300);
        cout << "future res = " << f3.get() << endl;
        std::future<int> f4 = threadpool_.Submit(&test_func_res_zero_arg);
        cout << "future res = " << f4.get() << endl;
        // function<void(int, int)> call([](int a, int b){
        //     cout << "function<void(int, int)>" << endl;
        // });
        // threadpool_.AddTask(call, 10, 100);

    }
private:
    int x_ = 10;
    ThreadPool threadpool_;
};

int main() {
    Test t;
    t.Add();
    return 0;
}