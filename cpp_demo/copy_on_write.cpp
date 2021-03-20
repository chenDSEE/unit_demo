// g++ test.cpp -lpthread -lgtest -lgtest_main -fsanitize=address -fno-omit-frame-pointer -g
// 这个测试会花比较长的时间来运行（带上了内存泄漏检测）

// 采用 智能指针（避免内存泄漏，保留旧的副本） + mutex（writer 优先，write 的时候，不允许新的 reader、writer） 实现 copy-on-write

#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <gtest/gtest.h>

using namespace std;
constexpr int INIT_SIZE = 10000;
constexpr int INSERT = 1000;
constexpr int TEST_ROUND = 3;

class DB {
public:
    explicit DB(int num)
        : DB_ptr(make_shared<unordered_map<int, int>>())
    {
        for (int i = 0; i < num; i++) {
            (*DB_ptr)[i] = i;
        }
    }

    void add(int num) {
        lock_guard<mutex> lock(DB_mtx);
        if (DB_ptr.use_count() != 1) {
            // copy-on-write
            auto tmp_ptr = make_shared<unordered_map<int, int>>(*DB_ptr);   // 把以前的数据继续保留
            DB_ptr.swap(tmp_ptr); // 因为正在 reader 的读者已经拿到了一个 ptr，没有开始的 reader 会被 DB_mtx 挡住，所以安全
        }

        // 增加数据
        int size = DB_ptr->size();
        for (int i = 0; i < num; i++) {
            (*DB_ptr)[size + i] = size + i;
        }
        cout << "update done[" << DB_ptr->size() << "]" << endl;
    }

    shared_ptr<unordered_map<int, int>> get_DB() const {    // 增加了引用计数
        lock_guard<mutex> lock(DB_mtx);
        return DB_ptr;
    }

private:
    mutable mutex DB_mtx;
    shared_ptr<unordered_map<int, int>> DB_ptr;
};

// ============  test start  ============

DB global_DB{INIT_SIZE};

void reader()
{
    for (int i = 0; i < TEST_ROUND; i++) {
        auto db = global_DB.get_DB();   // 在 get_DB 里面就已经保护好了，不需要再次保护

        int cnt = 0;
        for (auto itor : *db) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cnt++;
            EXPECT_EQ(itor.first, itor.second) << "[" << itor.first << "]:[" << itor.second << "]" << endl;
        }

        cout << "thread[" << this_thread::get_id() 
             << "], round[" << i 
             << "], size[" << db->size() 
             << "], count[" << cnt << "]" << endl;;
    }
}


// 多线程的 unit test 方法
TEST(FunTest, HandlesZeroInput) {
    thread r1(reader);
    thread r2(reader);
    thread r3(reader);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    global_DB.add(INSERT);   // writer
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    global_DB.add(INSERT);   // writer

    r1.join();
    r2.join();
    r3.join();
};

