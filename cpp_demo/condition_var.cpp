// g++ condition_var.cpp -lpthread -lgtest -lgtest_main -g -O0
// 这个测试会花比较长的时间来运行

#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <gtest/gtest.h>

using namespace std;
constexpr int TEST_TIME = 10000;

queue<int> mq;

// 所有的 锁、条件变量 都是为了保护 MQ 这个可以在不同线程之间相互访问的资源
condition_variable mq_cv;
mutex mq_mtx;

atomic<int> handle_cnt;

void producer()
{
    for (int i = 0; i < TEST_TIME; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        {
            lock_guard<mutex> lock(mq_mtx);
            mq.push(i);
        }
        mq_cv.notify_one();    // 这并不是对共享资源的操作，所以可以不在临界区里面
    }
}

void consumer()
{
    while (handle_cnt <= TEST_TIME && !mq.empty()) {
        // step 1: 加锁
        unique_lock<mutex> lock(mq_mtx);    // 只能用 unique_lock 来配合条件变量使用
        while (mq.empty()) {   // 避免虚假唤醒，step 2: 虚假唤醒检测
            mq_cv.wait(lock);   // step 3: 等待条件变量的通知
        }

#if 0
        // 采用 lambda 进行虚假唤醒处理
        unique_lock<mutex> lock(mq_mtx);
        mq_cv.wait(lock, [] {return !mq.empty();});
#endif

        EXPECT_FALSE(mq.empty());
        EXPECT_EQ(mq.front(), handle_cnt);

        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        handle_cnt++;
        mq.pop();
    }
}


// 多线程的 unit test 方法
TEST(FunTest, HandlesZeroInput) {
    std::thread t2(consumer);
    std::thread t3(consumer);
    std::thread t1(producer);

    t1.join(); 
    t2.join();
    t3.join();
};

