

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/time.h>


#define THREAD_SIZE		10


pthread_mutex_t mutex;
pthread_spinlock_t spinlock;

//mul_port_client_epoll.c
#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)


int inc(int *value, int add) {

	int old;
	
	__asm__ volatile (
		"lock; xaddl %2, %1;"
		: "=a" (old)
		: "m" (*value), "a" (add)
		: "cc", "memory"
	);

	return old;
}


// 10 * 100000
void *func (void *arg) {

	int *pcount = (int *)arg;

	int i = 0;
	while (i ++ < 1000000) {
#if 0
		(*pcount)++;

#elif 0

		pthread_mutex_lock(&mutex);
		(*pcount)++;
		pthread_mutex_unlock(&mutex);

#elif 0

		pthread_spin_lock(&spinlock);
		(*pcount)++;
		pthread_spin_unlock(&spinlock);

#elif 0

		int tmp = *pcount;
		cas(pcount, tmp, tmp+1);

#else

		inc(pcount, 1);



#endif
		//usleep(1);

	}

	
}

int main() {

	pthread_t threadid[THREAD_SIZE] = {0};


	pthread_mutex_init(&mutex, NULL);
	pthread_spin_init(&spinlock, PTHREAD_PROCESS_SHARED);

	struct timeval tv_start;

	gettimeofday(&tv_start, NULL);

	int i = 0;
	int count = 0;
	for (i = 0;i < THREAD_SIZE;i ++) {
		pthread_create(&threadid[i], NULL, func, &count);
	}
#if 1
	for (i = 0;i < THREAD_SIZE;i ++) {
		pthread_join(threadid[i], NULL); //
	}
#endif

	struct timeval tv_end;

	gettimeofday(&tv_end, NULL);

	int time_used = TIME_SUB_MS(tv_end, tv_start);
	printf("time_used: %d\n", time_used);

#if 0
	// 100w 
	for (i = 0;i < 100;i ++) {
		printf("count --> %d\n", count);
		sleep(1);
	}
#endif
}


