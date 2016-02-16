#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread1Print(void * params __attribute__ ((unused))){
	printf("I am thread 1\n");

	return NULL;
}

void* thread2Print(void * params __attribute__ ((unused))){
	printf("I am thread 2\n");

	return NULL;
}

int main(int argc __attribute__ ((unused)), char **argv __attribute__ ((unused))){
	int err = 0;
	void * ret;
	pthread_t t1;
	pthread_t t2;

	err = pthread_create(&t1, NULL, thread1Print, NULL);
	if(err) {
		fprintf(stderr, "ERR: pthread_create returned %d\n", err);
		exit(1);
	}

	err = pthread_create(&t2, NULL, thread2Print, NULL);
	if(err) {
		fprintf(stderr, "ERR: pthread_create returned %d\n", err);
		exit(1);
	}

	err = pthread_join(t1, &ret);
	if (err) {
		fprintf(stderr, "ERR: pthread_join returned %d\n", err);
		exit(1);
	}
	err = pthread_join(t2, &ret);
	if (err) {
		fprintf(stderr, "ERR: pthread_join returned %d\n", err);
		exit(1);
	}

	printf("I am thread 0\n");
	return 0;
}
