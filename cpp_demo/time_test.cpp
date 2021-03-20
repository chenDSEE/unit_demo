#include <iostream>
#include <chrono>
#include <thread>
#include <ratio>
using namespace std;

int main() {
    /* test */
    auto start = chrono::high_resolution_clock::now();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto end = chrono::high_resolution_clock::now();

    /* resultS */
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    cout<<"use "<<nanos.count()<<" nanos\n";

    std::chrono::duration<double> diff = end - start;
    cout<<"use "<<diff.count()<<" s\n";

//    秒精度
//    std::chrono::duration<double> diff = end - start;
//    cout<<"use "<<diff.count()<<" s\n";

//  纳秒精度
//    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
//    cout<<"use "<<nanos.count()<<" nanos\n";
    return 0;
}