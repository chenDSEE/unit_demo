#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>
#include <cassert>
#include <unordered_map>
#include <string>
 
std::atomic<std::string*> ptr;
int data;
 
void producer()
{
        std::cout << "in producer" << std::endl;

    std::string* p  = new std::string("Hello");
    data = 42;
    ptr.store(p, std::memory_order_release);
        std::cout << "out producer" << std::endl;

}
 
void consumer()
{
    std::cout << "in consumer" << std::endl;
    std::string* p2;
    while (!(p2 = ptr.load(std::memory_order_acquire))) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "consumer wait" << std::endl;
    }
        
    assert(*p2 == "Hello"); // 绝无问题
    assert(data == 42); // 绝无问题
    std::cout << "out consumer" << std::endl;

}
 
int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);

    std::cout << "test done" << std::endl;
    t1.join(); 
    t2.join();
}