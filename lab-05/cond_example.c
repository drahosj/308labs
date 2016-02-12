#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock;


void *printHello(void *args){

}

void *printCanYou(void *args){

}

void *printIWas(void *args){

}

int main(int argc, char **argv){
	int err;

	err = pthread_mutex_init(&lock, NULL);
	if(err){

	}

	return 0;
}
