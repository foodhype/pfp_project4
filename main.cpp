#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <papi.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <vector>


#define TOTAL_WORKLOAD 10000000
#define MIN_THREAD_COUNT 1
#define MAX_THREAD_COUNT 32
#define CACHE_MAX_SIZE 1000000000 // Used for clearing cache
#define LOOP_WARMUP_ITERATIONS 100000000 // Used for warming up program before taking measurements


int global_variable = 42;
std::atomic<int> atomic_global_variable(42);
std::mutex mtx;
volatile bool ready = false; 


void prepare() {
    // Warm up the system to ensure threads run on different cores.
    int warmup_loop_counter = 0;
    for (warmup_loop_counter = 0;
            warmup_loop_counter < LOOP_WARMUP_ITERATIONS;
            ++warmup_loop_counter) {
    }
    // Wait until main thread sets ready to true.
    while (!ready);
}


void read_global(int tid, long long workload) {
    prepare();
    for (long long i = 0; i < workload; ++i) {
        int temp = global_variable;
    }
}


void write_global(int tid, long long workload) {
    prepare();
    for (long long i = 0; i < workload; ++i) {
        global_variable = tid;
    }
}


void read_modify_write_global(int tid, long long workload) {
    prepare();
    for (long long i = 0; i < workload; ++i) {
        global_variable++;
    }
}


void atomic_read_modify_write_global(int tid, long long workload) {
    prepare();
    for (long long i = 0; i < workload; ++i) {
        atomic_global_variable++;
    }
}


void lock_unlock(int tid, long long workload) {
    prepare();
    for (long long i = 0; i < workload; ++i) {
        mtx.lock();
        mtx.unlock();
    }
}


void profile(const std::function<void(int, int)> &func, std::string func_name,
        long long workload, bool strong_scaling) {
    std::cout << func_name << std::endl;
    std::cout << "thread_count,runtime" << std::endl;
  
    for (int thread_count = MIN_THREAD_COUNT;
            thread_count <= MAX_THREAD_COUNT;
            ++thread_count) {

        // Clear all cached memory for consistent measurements.
        char cache[CACHE_MAX_SIZE];
        memset(cache, 0, sizeof cache);
        global_variable = rand();

        std::vector<std::thread> threads;
        for (int tid = 0; tid < thread_count; ++tid) {
            long_long thread_workload = strong_scaling ?
                    workload / thread_count :
                    workload; 
            threads.push_back(std::thread(func, tid, thread_workload));
        }

        sleep(1);
        long_long start_time_usec = PAPI_get_real_usec();
        ready = true;
        for (auto& t: threads) {
            t.join();
        }
        long_long end_time_usec = PAPI_get_real_usec();
        long_long elapsed_time_usec = end_time_usec - start_time_usec;
        ready = false;
	    
        /*
        std::cout << "name: " << func_name <<
                " thread count: " << thread_count <<
                " workload: " << workload <<
                " time elapsed (usec): " << elapsed_time_usec << std::endl;
	    */

	    std::cout << thread_count << "," << elapsed_time_usec << std::endl; 
    }

    std::cout << std::endl;
}


void record_measurements(bool strong_scaling) {
    if (strong_scaling) {
        std::cout << "\n\nSTRONG SCALING:\n\n" << std::endl;
    } else {
        std::cout << "\n\nWEAK SCALING:\n\n" << std::endl;
    }

    profile(read_global, "read_global", TOTAL_WORKLOAD, strong_scaling);
    profile(write_global, "write_global", TOTAL_WORKLOAD, strong_scaling);
    profile(read_modify_write_global, "read_modify_write_global", TOTAL_WORKLOAD, strong_scaling);
    profile(atomic_read_modify_write_global, "atomic_read_modify_write_global", TOTAL_WORKLOAD, strong_scaling);
    profile(lock_unlock, "lock_unlock", TOTAL_WORKLOAD, strong_scaling);
}


int main() {
    srand(time(NULL));
    record_measurements(true);
    record_measurements(false);

    return 0;
}
