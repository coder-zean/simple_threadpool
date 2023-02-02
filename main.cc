#include "thread_pool.h"
#include <iostream>
#include <functional>

using namespace std;

void test_func(int x) {
    cout << "test(int x)" << endl;
}

class TestCallable {
public:
    void operator()() {
        cout << "TestCallable::operator()()" << endl;
    }
};

class Test {
public:
    Test() : threadpool_(8) {}

    void Print() {
        cout << "Test::Print()" << endl;
    }

    void Add() {
        threadpool_.AddTask(this, &Test::Print);
        threadpool_.AddTask(&test_func, 10);
        TestCallable test_call;
        threadpool_.AddTask(test_call);
        function<void(int, int)> call([](int a, int b){
            cout << "function<void(int, int)>" << endl;
        });
        threadpool_.AddTask(call, 10, 100);

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