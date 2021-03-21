// g++ test.cpp
// 检测：ls /proc/`pgrep a.out`/task/

#include <thread>
#include <iostream>
#include <chrono>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <gtest/gtest.h>

using namespace std;


// ============  header file(xxx.h)  ============
#include<sys/syscall.h>

namespace NR
{
/*
正常流程是：
.cpp: __thread int local_tid = 0;
.h  : extern __thread int local_tid;

这里由于编译问题，暂且不这样做
*/
__thread int local_tid = 0;  // 会在每一个线程都创建一个，每个线程都只会用自己的那个。需要自己在 .cpp 里面定义一下


pid_t get_tid()
{
    // man 2 gettid
    // There is no glibc wrapper for this system call; see NOTES.
    // Glibc does not provide a wrapper for this system call; call it using syscall(2).
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

int cahce_tid()
{
    if (local_tid == 0) {
        local_tid = get_tid();
    }
}

inline int tid()
{
    // __thread 本身就是这个线程唯一的，所以不会有什么 race condition 的
    if (__builtin_expect(local_tid == 0, 0)) {
        // local_tid == 0 这件事发生的概率很小，我们基本不认为会发生，用 unlikely 也可以
        cahce_tid();
    }
    return local_tid;
}

}   // namespace NR


// ============  source file(xxx.cpp)  ============
void do_work()
{
    cout << "[" << NR::tid() << "]" << endl;
    std::this_thread::sleep_for(std::chrono::seconds(100));
}

int main()
{
    std::cout << NR::tid() << std::endl;
    thread t1(do_work);
    thread t2(do_work);
    thread t3(do_work);
    thread t4(do_work);
    thread t5(do_work);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    return 0;
}
