// 主线程等待全部子线程启动后在继续运行
// 采用 条件变量 + 锁 来实现

#include <thread>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std;

// 作为主线程跟其他子线程启动期间的同步器
class CountDownLatch {
public:
    explicit CountDownLatch(int times)
        : counter(times), counter_mtx(), thread_cv()
    {
        // counter_mtx(),thread_cv(counter_mtx) 顺序不可以颠倒，确保 cv 顺利初始化
    }

    void count_down() {
        lock_guard<mutex> lock(counter_mtx);
        counter--;
        cout << "count down one [" << counter << "]" << endl;
        if (counter == 0) { // wait 的线程可以开始干活了
            thread_cv.notify_all();
        }
    }

    void wait() {
        cout << "main thread start to wait" << endl;

        unique_lock<mutex> lock(counter_mtx);
        thread_cv.wait(lock, [this]{return counter == 0;}); // 等待 count down 0 的通知
    }

    int get_count() {
        lock_guard<mutex> lock(counter_mtx);
    }

public:
    CountDownLatch(CountDownLatch& rhs) = delete;
    CountDownLatch& operator= (const CountDownLatch& rhs) = delete;

private:
    int counter;        // 由 counter_mtx 保护，thread_cv 进行启动通知
    mutable mutex counter_mtx;  // 保护 counter 的计数
    condition_variable thread_cv;    // 通知其他线程
};

void fun(CountDownLatch& ctrl)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ctrl.count_down();
}

int main()
{
    CountDownLatch ctrl{3};
    thread w1(fun, std::ref(ctrl)); // 只能这样传引用了
    thread w2(fun, std::ref(ctrl));
    thread w3(fun, std::ref(ctrl));
    w1.detach();
    w2.detach();
    w3.detach();

    ctrl.wait();

    cout << "all done main can start now" << endl;
    return 0;
}
