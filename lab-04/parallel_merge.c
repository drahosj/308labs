#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>

// setting a maximum number of threads
#define MAX_THREADS 512

// making sure that the maximum malloc size is not reached
#define MAX_ARR_ELEMENTS 32768

// True if verbose output is to be given, false otherwise
int verbose_flag = 0;

// struct to keep track of sub-arrays that will be sorted
struct sub_array{
	int position;
	int s_step;
	int length;
	int *array;
};

/**
 * merges all the elements
 */
int merge(struct sub_array * s_array, int sub_array_start ,int sub_array_end, int * array, int array_start, int array_end){
	int i;
	int j;
	int a_pos;
	int b_pos;
	if((sub_array_end - sub_array_start) > 2){
		// merge left
		merge(s_array, sub_array_start ,sub_array_end / 2, array, array_start, array_end / 2);
		// merge right
		merge(s_array, sub_array_end / 2 + 1, sub_array_end, array, array_end / 2 + 1, array_end);
		for(i = array_start; i < array_end; i++){
			
		}
	}else{
		i = sub_array_start;
		j = array_start;
		if(verbose_flag) printf("merging subarrays %d and %d\n", i, i + 1);
		a_pos = 0;
		b_pos = 0;
		while(a_pos < s_array[i].length && b_pos < s_array[i + 1].length){
			if(s_array[i].array[a_pos] <= s_array[i + 1].array[b_pos]){
				if(verbose_flag) printf("element %d of sub-array %d is less than or equal to element %d of sub-array %d\n", a_pos, i, b_pos, i + 1);
				array[j] = s_array[i].array[a_pos];
				if(verbose_flag) printf("element %d of `array` is: %d\n", j, array[j]);
				j++;
				a_pos++;
			}else{
				if(verbose_flag) printf("element %d of sub-array %d is less than or equal to element %d of sub-array %d\n", b_pos, i + 1, a_pos, i);
				array[j] = s_array[i + 1].array[b_pos];
				if(verbose_flag) printf("element %d of `array` is: %d\n", j, array[j]);
				j++;
				b_pos++;
			}
		}
		if(a_pos == s_array[i].length){
			while(b_pos < s_array[i + 1].length){
				if(verbose_flag) printf("the sub-array %d has been exhausted\n", i);
				array[j] = s_array[i + 1].array[b_pos];
				if(verbose_flag) printf("element %d of `array` is: %d\n", j, array[j]);
				j++;
				b_pos++;
			}
		}else{
			while(a_pos < s_array[i].length){
				if(verbose_flag) printf("the sub-array %d has been exhausted\n", i + 1);
				array[j] = s_array[i].array[a_pos];
				if(verbose_flag) printf("element %d of `array` is: %d\n", j, array[j]);
				j++;
				a_pos++;
			}
		}
	}
	return 0;
}

/**
 * sorts the sub-array using insertion sort
 */
void * thread_sort(void * s){
	struct sub_array * sub = (struct sub_array *)s;
	int i;
	int j;
	for(i = 0; i < sub->length; i++){
		j = i;
		while(j > 0 && (sub->array[j - 1] > sub->array[j])){
			flockfile(stdout);
			if(verbose_flag) printf("thread %d: j = %d; array[%d] = %d; array[%d] = %d\n", sub->position, j, j - 1,sub->array[j - 1], j, sub->array[j]);
			funlockfile(stdout);

			sub->array[j - 1] = sub->array[j - 1] ^ sub->array[j];
			sub->array[j] = sub->array[j - 1] ^ sub->array[j];
			sub->array[j - 1] = sub->array[j - 1] ^ sub->array[j];

			flockfile(stdout);
			if(verbose_flag) printf("thread %d: j = %d; array[%d] = %d; array[%d] = %d\n", sub->position, j, j - 1, sub->array[j - 1], j, sub->array[j]);
			funlockfile(stdout);

			j--;
		}
	}
}

/**
 * 
 */
int main(int argc, char **argv){
	int *arr;
	int c;
	int i;
	int j;
	int arr_pos = 0;
	int step;
	int num_elements = 2 * MAX_THREADS;
	unsigned int rand_seed = 42;
	struct sub_array * sub_arrs;
	FILE * in_array;
	FILE * out_array;
	pthread_t * threads;
	
	// parse the command line arguments
	c = getopt(argc, argv, "n:vs:");
	while(c != -1){
		switch(c){
			// n is the number of elements in the array, default is 1024
			case 'n':
				num_elements = atoi(optarg);
				if(num_elements > MAX_ARR_ELEMENTS){
					num_elements = MAX_ARR_ELEMENTS;
				}
				break;
			// v is for verboseness
			case 'v':
				verbose_flag = 1;
				break;
			// s is to set the seed for the random number generator. Default is 42
			case 's':
				rand_seed = atoi(optarg);
				break;
			default:
				abort();
		}
		c = getopt(argc, argv, "n:vs:");
	}

	// calculate the step(or size of sub-array)
	if(0 == num_elements % MAX_THREADS){
		step = num_elements / MAX_THREADS;
	}else{
		step = (num_elements / MAX_THREADS) + 1;
	}
	if(verbose_flag) printf("the value of step is calculated to be: %d\n", step);

	// first, we want to set the random number seed
	srand(rand_seed);
	if(verbose_flag) printf("set the random seed\n");
	
	// allocate the array
	arr = calloc(num_elements, sizeof(int));
	if(NULL == arr){
		perror("calloc encountered an error:");
		return -1;
	}else{
		if(verbose_flag) printf("array `arr` is allocated\n");
	}

	// open the file `in_array.dat`
	in_array = fopen("./in_array.dat", "w");
	if(NULL == in_array){
		perror("fopen");
		free(arr);
		return -1;
	}else{
		if(verbose_flag) printf("the file `in_array.dat` opened successfully\n");
	}

	// populate the array, and print it to `in_array`
	for(i = 0; i < num_elements; i++){
		arr[i] = rand() % num_elements;
		if(0 == (i % 16)){
			fprintf(in_array, "%d,", arr[i]);
		}else if(15 == (i % 16) || (i + 1 == num_elements)){
			fprintf(in_array, " %d\n", arr[i]);
		}else{
			fprintf(in_array, " %d,", arr[i]);
		}
	}

	// once printing is complete, close the file
	fclose(in_array);

	// allocate the `sub_array` structs
	sub_arrs = calloc(MAX_THREADS, sizeof(struct sub_array));
	if(NULL == sub_arrs){
		perror("calloc encountered an error:");
		free(arr);
		return -1;
	}else{
		if(verbose_flag) printf("the array `sub_arrs` is allocated\n");
	}

	// populate the elements in the `sub_arrs` array
	for(i = 0; i < MAX_THREADS; i++){
		sub_arrs[i].position = i;
		sub_arrs[i].s_step = step;
		if(verbose_flag) printf("the position and step of element %d is set\n", i);
		if((num_elements - arr_pos) >= step){
			sub_arrs[i].array = calloc(step, sizeof(int));
			if(NULL == sub_arrs[i].array){
				perror("calloc encountered an error:");
				free(sub_arrs);
				free(arr);
				return -1;
			}
			sub_arrs[i].length = step;
		}else{
			sub_arrs[i].array = calloc((num_elements - arr_pos), sizeof(int));
			if(NULL == sub_arrs[i].array){
				perror("calloc encountered an error:");
				free(sub_arrs);
				free(arr);
				return -1;
			}
			sub_arrs[i].length = (num_elements - arr_pos);
		}
		if(verbose_flag) printf("the array `array` for the %dth element of `sub_arrs` is allocated, and length is set to %d\n", i, sub_arrs[i].length);
		j = 0;
		while((j < step) && (arr_pos < num_elements)){
			sub_arrs[i].array[j] = arr[arr_pos++];
			if(verbose_flag) printf("element %d of `array` is %d\n", j, sub_arrs[i].array[j]);
			j++;
		}
	}

	// Time to sort this array in parallel
	// allocate the `threads` array
	threads = calloc(MAX_THREADS, sizeof(pthread_t));
	if(NULL == threads){
		perror("calloc encountered an error:");
		free(sub_arrs);
		free(arr);
		return -1;
	}else{
		if(verbose_flag) printf("the array `threads` is allocated\n");
	}

	// create the threads to sort the array
	for(i = 0; i < MAX_THREADS; i++){
		pthread_create(&threads[i], NULL, thread_sort, &sub_arrs[i]);
	}

	// join the threads
	for(j = 0; j < MAX_THREADS; j++){
		pthread_join(threads[j], NULL);
	}

	// once all threads terminate, we do the merging
	merge(sub_arrs, MAX_THREADS, arr, num_elements);

	// Once merging is complete, we print out the new array
	// first, we need to open the file
	out_array = fopen("./out_array.dat", "w");
	if(NULL == out_array){
		perror("fopen encountered an error:");
		free(sub_arrs);
		free(arr);
		free(threads);
		return -1;
	}else{
		if(verbose_flag) printf("the file `out_array.dat` is opened successfully\n");
	}

	// now we print to the file
	for(i = 0; i < num_elements; i++){
		if(0 == (i % 16)){
			fprintf(out_array, "%d,", arr[i]);
		}else if(15 == (i % 16) || (i + 1 == num_elements)){
			fprintf(out_array, " %d\n", arr[i]);
		}else{
			fprintf(out_array, " %d,", arr[i]);
		}
	}

	// clean up all the allocations
	free(arr);
	free(sub_arrs);
	free(threads);

	return 0;
}
