#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <papi.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <vector>


#define TOTAL_WORKLOAD 10000000
#define MIN_THREAD_COUNT 1
#define MAX_THREAD_COUNT 100
#define CACHE_MAX_SIZE 1000000000 // Used for clearing cache
#define LOOP_WARMUP_ITERATIONS 1000000000 // Used for warming up program before taking measurements


int global_variable = 42;
std::atomic<int> atomic_global_variable(42);
std::mutex mtx;


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


void read_modify_write_global(int tid, long long workload) {
    for (long long i = 0; i < workload; ++i) {
        global_variable++;
    }
}


void atomic_read_modify_write_global(int tid, long long workload) {
    for (long long i = 0; i < workload; ++i) {
        atomic_global_variable++;
    }
}


void lock_unlock(int tid, long long workload) {
    for (long long i = 0; i < workload; ++i) {
        mtx.lock();
        mtx.unlock();
    }
}


void profile(const std::function<void(int, int)> &func, std::string func_name,
        long long workload) {
    // Warm up the system to ensure threads run on different cores.
    int warmup_loop_counter = 0;
    for (warmup_loop_counter = 0;
            warmup_loop_counter < LOOP_WARMUP_ITERATIONS;
            ++warmup_loop_counter) {
    }

    for (int thread_count = MIN_THREAD_COUNT;
            thread_count <= MAX_THREAD_COUNT;
            ++thread_count) {
        // Clear all cached memory for consistent measurements.
        char cache[CACHE_MAX_SIZE];
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
    profile(read_modify_write_global, "read_modify_write_global", TOTAL_WORKLOAD);
    profile(atomic_read_modify_write_global, "atomic_read_modify_write_global", TOTAL_WORKLOAD);
    profile(lock_unlock, "lock_unlock", TOTAL_WORKLOAD);

    return 0;
}
