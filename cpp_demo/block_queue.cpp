// 有部分为了 gtest 的入侵式修改，不得已而为之
// g++ test.cpp -lpthread -lgtest -lgtest_main -fsanitize=address -fno-omit-frame-pointer -g
// 这个测试会花比较长的时间来运行（带上了内存泄漏检测）


#include <thread>
#include <iostream>
#include <chrono>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <gtest/gtest.h>

using namespace std;

// multi-thread safe
// 向里面填充指针式更明智的做法
template <typename T>
class MessageQueue
{
public:
    explicit MessageQueue() : mtx(), cv() {}

    void push(const T& msg) {
        lock_guard<mutex> lock(mtx);
        mq.emplace(msg);
        cv.notify_one();
    }

    void push(T&& msg) {
        lock_guard<mutex> lock(mtx);
        mq.emplace(std::move(msg));
        cv.notify_one();
    }

    T pop() {   // 有 message 的话直接 pop，没有的话阻塞等待
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&]{ return !mq.empty();});    // 避免虚假唤醒

        T output{std::move(mq.front())};    // mq.front() 本身返回的就是一个右值；TODO: move 还有必要吗？
                                            // move() 是必要的，出来的并不是右值
        mq.pop();
        pop_cnt++;
        return output;  // 会有返回值优化，没关系
    }

    size_t size() const {
        lock_guard<mutex> lock(mtx);
        return mq.size();
    }

    int get_pop_cnt() { // for gtset
        lock_guard<mutex> lock(mtx);
        return pop_cnt;
    }

public:
    MessageQueue(const MessageQueue& rhs) = delete;
    MessageQueue& operator= (const MessageQueue& rhs) = delete;

private:
    int pop_cnt = 0;    // for gtest
    mutable mutex mtx;
    condition_variable cv;
    queue<T> mq;
};

// ============  test start  ============
#if 0
class test
{
public:
    test() {
        cout << "defalut ctor, this[" << this << "]" << endl;
    }

    test(const test& rhs) {
        cout << "copy ctor, from[" << &rhs <<"] to this[" << this << "]" << endl;
    }

    test(test&& rhs) {
        cout << "move ctor, from[" << &rhs <<"] to this[" << this << "]" << endl;
    }
};

int main()
{
    MessageQueue<test> MQ;
    MQ.push(test{});    // 移动构造
    
    test t1;
    MQ.push(t1);        // 拷贝构造

    cout << "==== pop size[" << MQ.size() <<"] ===" << endl;

    MQ.pop();           // pop() 会调用 移动构造
    test t2 = MQ.pop();

    return 0;
}

#else

MessageQueue<unique_ptr<int>> MQ;
constexpr int INIT_SIZE = 10000;
constexpr int APPEND_SIZE_1 = 10000;
constexpr int APPEND_SIZE_2 = 10000;



void consumer() {
    while (1) {
        unique_ptr<int> task = MQ.pop();

        if (MQ.get_pop_cnt() == (INIT_SIZE + APPEND_SIZE_2 + APPEND_SIZE_1)) {
            // to stop and finish gtest
            cout << "====== all test done and success, place stop by yourself ======" << endl;
            break;
        }
    }

}


void producer() {
    for (int i = 0; i < INIT_SIZE; i++) {
        MQ.push(make_unique<int>(i));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    cout << "round two" << endl;
    for (int i = INIT_SIZE; i < APPEND_SIZE_1 + INIT_SIZE; i++) {
        MQ.push(make_unique<int>(i));
    }

    cout << "round three" << endl;
    for (int i = APPEND_SIZE_1 + INIT_SIZE; i < APPEND_SIZE_1 + APPEND_SIZE_2 + INIT_SIZE; i++) {
        MQ.push(make_unique<int>(i));
    }

    cout << "producer done" << endl;
}


// 多线程的 unit test 方法
TEST(FunTest, HandlesZeroInput) {
    thread c1(consumer);
    thread c2(consumer);
    thread c3(consumer);
    thread p1(producer);


    p1.join();
    c1.join();
    c2.join();
    c3.join();
    cout << "test done" << endl;
};


#endif
