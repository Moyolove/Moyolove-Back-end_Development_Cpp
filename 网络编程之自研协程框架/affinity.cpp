#include<pthread.h>
#include<iostream>

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>


#define THREAD_COUNT 2

void *thread_func(void* arg){
    int thread_id = *(int *)arg;
    std::cout << "Thread " << thread_id << " runs on cpu" << std::endl;
    while(1);
}


int main(){
    pthread_t threads[THREAD_COUNT];
    int thread_id[THREAD_COUNT];

    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    for(int i = 0; i < THREAD_COUNT; i++){
        CPU_SET(i, &cpus);
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        thread_id[i] = i;
        pthread_create(&threads[i], NULL, thread_func, (void *)&thread_id[i]);
        pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpus);
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}