#include <functional>
#include <iostream>
#include <papi.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <vector>


#define TOTAL_WORKLOAD 1000000000
#define MIN_THREAD_COUNT 1
#define MAX_THREAD_COUNT 100


int global_variable = 42;


void read_global(int tid, long long workload) {
    for (long long i = 0; i < workload; ++i) {
        int temp = global_variable;
    }
}


void write_global(int tid, long long workload) {
    for (long long i = 0; i < workload; ++i) {
        global_variable = tid;
    }
}


void profile(const std::function<void(int, int)> &func, std::string func_name,
        long long workload) {
    for (int thread_count = MIN_THREAD_COUNT;
            thread_count <= MAX_THREAD_COUNT;
            ++thread_count) {
        // Clear cache and reset global variable for consistent measurements.
        char cache[1000000000];
        memset(cache, 0, sizeof cache);
        global_variable = rand();

        long_long start_time_usec = PAPI_get_real_usec();

        std::vector<std::thread> threads;
        for (int tid = 0; tid < thread_count; ++tid) {
            threads.push_back(std::thread(func, tid, workload / thread_count));
        }
        for (auto& t: threads) {
            t.join();
        }

        long_long elapsed_time_usec = PAPI_get_real_usec() - start_time_usec;

        std::cout << "name: " << func_name <<
                " thread count: " << thread_count <<
                " workload: " << workload <<
                " time elapsed (usec): " << elapsed_time_usec << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    srand(time(NULL));
    profile(read_global, "read_global", TOTAL_WORKLOAD);
    profile(write_global, "write_global", TOTAL_WORKLOAD);

    return 0;
}
