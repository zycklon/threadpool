#define THREAD 9
#define QUEUE  9999999

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"

int tasks = 0;
int calls = 0;
pthread_mutex_t lock;
threadpool_t *pool;
long answer = 0;

void fibonacci_task(void *arg) {

	int nr = (int) arg;
	threadpool_error_t err;

	pthread_mutex_lock(&lock);
	calls++;
	pthread_mutex_unlock(&lock);

	if(nr == 0 || nr == 1) {
		pthread_mutex_lock(&lock);
		answer += nr;
		pthread_mutex_unlock(&lock);
	} else {

		if((err = threadpool_add_task(pool, &fibonacci_task, (void *)(nr-2), 0)) == 0) {
			pthread_mutex_lock(&lock);
				tasks++;
			pthread_mutex_unlock(&lock);
		}
		else {
			fprintf(stdout, "errno: %d\n", err);
			threadpool_destroy(pool, 0);
			exit(err);
		}

		if( (err = threadpool_add_task(pool, &fibonacci_task, (void *)(nr-1), 0)) == 0 ) {
			pthread_mutex_lock(&lock);
				tasks++;
			pthread_mutex_unlock(&lock);
		}
		else {
			fprintf(stdout, "errno: %d\n", err);
			threadpool_destroy(pool, 0);
			exit(err);
		}
	}
}

int main(int argc, char **argv)
{
	int n = 0;

    if(argc == 2)
        n = atoi(argv[1]);
    else {
        printf("usage: fibonacci number\n");
        exit(0);
    }

    pthread_mutex_init(&lock, NULL);

    assert((pool = threadpool_create(THREAD, QUEUE, 0)) != NULL);
    fprintf(stderr, "Pool started with %d threads and "
            "queue size of %d\n", THREAD, QUEUE);

    assert(threadpool_add_task(pool, &fibonacci_task, (void *)n, 0) == 0);

    pthread_mutex_lock(&lock);
    	tasks++;
    pthread_mutex_unlock(&lock);

    while(tasks != calls) {
        usleep(10000);
    }

    assert(threadpool_destroy(pool, 0) == 0);
    fprintf(stdout, "Processed %d tasks - fib(%d): %ld\n", tasks, n, answer);

    return 0;
}
