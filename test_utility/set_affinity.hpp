#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>

void set_affinity(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

// 在线程函数开头调用
// set_affinity(thread_id % num_cores);