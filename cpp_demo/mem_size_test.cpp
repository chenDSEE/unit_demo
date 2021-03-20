#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

// echo 3 > /proc/sys/vm/drop_caches
// grep Pss /proc/[1-9]*/smaps | awk '{total+=$2}; END {printf "%d kB\n", total }'     // total mem
// pidstat -r -p <pid>

constexpr uint32_t PER_BYTE = sizeof(char);
constexpr uint32_t PER_KB = 1024 * PER_BYTE;
constexpr uint32_t PER_MB = 1024 * PER_KB;
constexpr uint32_t PER_GB = 1024 * PER_MB;

constexpr uint32_t SET = 1000;  // set this
constexpr uint32_t TOTAL_SIZE = SET * PER_GB;

int main()
{
    char* ptr = new char[TOTAL_SIZE];
    for (int i = 0; i < TOTAL_SIZE; i++) {
        ptr[i] = 'a';   // make system alloc heap in RSS, but not VIRT
    }

    std::this_thread::sleep_for(std::chrono::seconds(100));
    return 0;
}
