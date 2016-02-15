#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define MAX_PRODUCE 40

sem_t buffer_semaphore;

// Producer thread
void *producer(void *args){
	// Produces ints and puts them into the buffer
	// until the value of `buffer_semaphore` gets
	// to 20, then wait for the consumer to consume
	// the ints
	int i = 0;
	int sem_val;
	int err;
	int tail = 0;
	int *buffer = (int *)args;
	while(i < MAX_PRODUCE){
		err = sem_getvalue(&buffer_semaphore, &sem_val);
		if(err){
			perror("sem_getvalue");
			pthread_exit(NULL);
		}

		if(sem_val < 20){
			buffer[tail % 20] = rand() % MAX_PRODUCE;
			flockfile(stdout);
			fprintf(stdout, "Producer: element %d: %d; placed in the tail of the buffer\n", i, buffer[tail & 20]);
			funlockfile(stdout);
			i++;
			sem_post(&buffer_semaphore);
			tail++;
		}
	}
}

// Consumer thread
void *consumer(void *args){
	// Consumes the ints that are put into the buffer
	// by the producer until either all the ints produced
	// by the Producer is consumed, or the buffer is empty
	int i = 0;
	int sem_val;
	int err;
	int head = 0;
	int *buffer = (int *)args;
	while(i < MAX_PRODUCE){
		err = sem_getvalue(&buffer_semaphore, &sem_val);
		if(err){
			perror("sem_getvalue");
			pthread_exit(NULL);
		}

		if(sem_val > 0){
			flockfile(stdout);
			fprintf(stdout, "Consumer: element %d: %d; read from the head of the buffer\n", i, buffer[head % 20]);
			funlockfile(stdout);
			buffer[head % 20] = -1;
			i++;
			sem_wait(&buffer_semaphore);
			head++;
		}
	}
}

int main(int argc, char **argv){
	int err;
	int *buffer;
	pthread_t t1;
	pthread_t t2;

	buffer = calloc(20, sizeof(int));
	if(NULL == buffer){
		perror("calloc");
		return -1;
	}

	err = sem_init(&buffer_semaphore, 0, 0);
	if(err){
		perror("sem_init");
		free(buffer);
		return -1;
	}

	err = pthread_create(&t1, NULL, producer, buffer);
	if(err){
		errno = err;
		sem_destroy(&buffer_semaphore);
		free(buffer);
		return -1;
	}

	err = pthread_create(&t2, NULL, consumer, buffer);
	if(err){
		errno = err;
		sem_destroy(&buffer_semaphore);
		free(buffer);
		return -1;
	}

	err = pthread_join(t1, NULL);
	if(err){
		errno = err;
		perror("pthread_join");
		sem_destroy(&buffer_semaphore);
		free(buffer);
		return -1;
	}

	err = pthread_join(t2, NULL);
	if(err){
		errno = err;
		perror("pthread_join");
		sem_destroy(&buffer_semaphore);
		free(buffer);
		return -1;
	}

	return 0;
}
