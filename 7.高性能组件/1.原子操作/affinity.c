ABCDEFGHIJKLMNOPQRSTE


#include <stdio.h>
#include <unistd.h>
#include <sched.h>

#include <sys/syscall.h>


// nginx.conf --> set affinity 0000 0000
void process_affinity(int num) {

	
	pid_t self_id = syscall(__NR_gettid);

	//fd_set
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(self_id % num, &mask);

	sched_setaffinity(self_id, sizeof(mask), &mask);
	
	while(1) usleep(1);
}


int main() {

	int num = sysconf(_SC_NPROCESSORS_CONF);

	printf("num: %d\n", num);
#if 0

	int i = 0;
	pid_t pid = 0;
	
	for (i = 0;i < num/2;i ++) {

		pid = fork();
		if (pid <= 0) {
			break;
		}

	}

	if (pid == 0) {
		process_affinity(num);
	}

#else

	int i = 0;
	
	for (i = 0;i < num/2;i ++) {
			
		pthread_create();
			
	}

#endif
	printf("affinity.c: %d\n", pid);

	while(1) usleep(1);

}



