#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <papi.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <thread>
#include <vector>


#define TOTAL_WORKLOAD 10000000
#define MIN_THREAD_COUNT 1
#define MAX_THREAD_COUNT 31
#define CACHE_MAX_SIZE 1000000000 // Used for clearing cache
#define LOOP_WARMUP_ITERATIONS 1000000000 // Used for warming up program before taking measurements


int global_variable = 42;
std::atomic<int> atomic_global_variable(42);
std::mutex mtx;
volatile std::atomic<bool> ready(false); 

void read_global(int tid, long long workload) {
    while (!ready);
    for (long long i = 0; i < workload; ++i) {
        int temp = global_variable;
    }
}


void write_global(int tid, long long workload) {
    while (!ready);
    for (long long i = 0; i < workload; ++i) {
        global_variable = tid;
    }
}


void read_modify_write_global(int tid, long long workload) {
    while (!ready);
    for (long long i = 0; i < workload; ++i) {
        global_variable++;
    }
}


void atomic_read_modify_write_global(int tid, long long workload) {
    while (!ready);
    for (long long i = 0; i < workload; ++i) {
        atomic_global_variable++;
    }
}


void lock_unlock(int tid, long long workload) {
    while (!ready);
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
  
  /*
  cpu_set_t set;
  CPU_ZERO (&set);
  CPU_SET(16, &set);
  if (sched_setaffinity(getpid(), sizeof (cpu_set_t), &set))
    {
      std::cerr << "Error!!";
      return;
    }
  */
    std::cout << func_name << std::endl;
    std::cout << "thread_count,runtime" << std::endl;
  
    for (int thread_count = MIN_THREAD_COUNT;
            thread_count <= MAX_THREAD_COUNT;
            ++thread_count) {
        // Clear all cached memory for consistent measurements.
        char cache[CACHE_MAX_SIZE];
        memset(cache, 0, sizeof cache);
        global_variable = rand();

        //long_long start_time_usec = PAPI_get_real_usec();

        std::vector<std::thread> threads;
        for (int tid = 0; tid < thread_count; ++tid) {
            threads.push_back(std::thread(func, tid, workload / thread_count));
        }
	long_long start_time_usec = PAPI_get_real_usec();
	ready = true;
        for (auto& t: threads) {
            t.join();
        }

        long_long elapsed_time_usec = PAPI_get_real_usec() - start_time_usec;
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

int main() {
    srand(time(NULL));
    profile(read_global, "read_global", TOTAL_WORKLOAD);
    profile(write_global, "write_global", TOTAL_WORKLOAD);
    profile(read_modify_write_global, "read_modify_write_global", TOTAL_WORKLOAD);
    profile(atomic_read_modify_write_global, "atomic_read_modify_write_global", TOTAL_WORKLOAD);
    profile(lock_unlock, "lock_unlock", TOTAL_WORKLOAD);

    return 0;
}
