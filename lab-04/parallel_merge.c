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
	int i_n;
	int j;
	int j_n;
	int k;
	int a_pos;
	int b_pos;
	int c_pos;
	int array_length = array_end - array_start;
	// variable to tell when to sort from `temp_array` to `array`
	int t = 0;
	int * temp_array;

	temp_array = calloc(array_length, sizeof(int));
	if(NULL == temp_array){
		perror("calloc encountered an error");
		return -1;
	}else{
		if(verbose_flag) printf("array `temp_array` allocated\n");
	}

	for(i = 1; i < (sub_array_end - sub_array_start); i = i << 1){
		if(verbose_flag) printf("in outer for loop, i = %d\n", i);
		if(i == 1){
			j = 0;
			for(i_n = 0; i_n < (sub_array_end - sub_array_start); i_n = i_n + (i << 1)){
				if(verbose_flag) printf("in inner for loop, i_n = %d\n", i_n);
				a_pos = 0;
				b_pos = 0;
				// traverse through the `array`s in the adjacent elements in the `sub_array` array
				// and do the merge into `temp_array`
				while(a_pos < s_array[i_n].length && b_pos < s_array[i_n + 1].length){
					if(s_array[i_n].array[a_pos] <= s_array[i_n + 1].array[b_pos]){
						if(verbose_flag) printf("element %d of sub-array %d is less than or equal to element %d of sub-array %d\n", a_pos, i_n, b_pos, (i_n + 1));
						temp_array[j] = s_array[i_n].array[a_pos];
						if(verbose_flag) printf("temp_array[%d] = %d\n", j, temp_array[j]);
						j++;
						a_pos++;
					}else{
						if(verbose_flag) printf("element %d of sub-array %d is less than or equal to element %d of sub-array %d\n", b_pos, (i_n + 1), a_pos, i_n);
						temp_array[j] = s_array[i_n + 1].array[b_pos];
						if(verbose_flag) printf("temp_array[%d] = %d\n", j, temp_array[j]);
						j++;
						b_pos++;
					}
				}

				if(a_pos >= s_array[i_n].length){
					if(verbose_flag) printf("sub-array %d has been exhausted\n", i_n);
					while(b_pos < s_array[i_n + 1].length){
						temp_array[j] = s_array[i_n + 1].array[b_pos];
						if(verbose_flag) printf("temp_array[%d] = %d\n", j, temp_array[j]);
						j++;
						b_pos++;
					}
				}else{
					if(verbose_flag) printf("sub-array %d has been exhausted\n", (i_n + 1));
					while(a_pos < s_array[i_n].length){
						temp_array[j] = s_array[i_n].array[a_pos];
						if(verbose_flag) printf("temp_array[%d] = %d\n", j, temp_array[j]);
						j++;
						a_pos++;
					}
				}
			}
			t = !t;
		}else if(t){
			// merge from `temp_array` into `array`
			for(i_n = 0; i_n < array_length; i_n = i_n + ((i * s_array[0].length) << 1)){
				if(verbose_flag) printf("in inner loop, i_n = %d\n", i_n);
				a_pos = i_n;
				// if i_n + i is greater than array length, then pick array length, otherwise pick i_n + i
				b_pos = (i_n + (i * s_array[0].length)) > (array_length) ? (array_length) : (i_n + (i * s_array[0].length));
				j_n = b_pos;
				// if i_n + (i << 1) is greater than array length, then pick array length, otherwise pick i_n + (i << 1)
				c_pos = (i_n + ((i * s_array[0].length) << 1)) > (array_length) ? (array_length) : (i_n + ((i * s_array[0].length) << 1));
				if(verbose_flag) printf("a_pos = %d; b_pos = %d; c_pos = %d\n", a_pos, b_pos, c_pos);
				for(j = a_pos; j < (i_n + ((i * s_array[0].length) << 1)); j++){
					if((a_pos < j_n) && ((b_pos >= c_pos) || (temp_array[a_pos] <= temp_array[b_pos]))){
						if(verbose_flag) printf("element %d of `temp_array` is less than or equal to element %d of `temp_array`\n", a_pos, b_pos);
						array[j] = temp_array[a_pos];
						a_pos++;
						if(verbose_flag) printf("array[%d] = %d\n", j, array[j]);
					}else{
						if(verbose_flag) printf("element %d of `temp_array` is less than or equal to element %d of `temp_array`\n", b_pos, a_pos);
						array[j] = temp_array[b_pos];
						b_pos++;
						if(verbose_flag) printf("array[%d] = %d\n", j, array[j]);
					}
				}
			}
			t = !t;
		}else{
			// merge from `array into `temp_array`
			for(i_n = 0; i_n < array_length; i_n = i_n + ((i * s_array[0].length) << 1)){
				if(verbose_flag) printf("in inner loop, i_n = %d\n", i_n);
				a_pos = i_n;
				// if i_n + i is greater than array length, then pick array length, otherwise pick i_n + i
				b_pos = (i_n + (i * s_array[0].length)) > (array_length) ? (array_length) : (i_n + (i * s_array[0].length));
				j_n = b_pos;
				// if i_n + (i << 1) is greater than array length, then pick array length, otherwise pick i_n + (i << 1)
				c_pos = (i_n + (i << 1)) > (array_length) ? (array_length) : (i_n + ((i * s_array[0].length) << 1));
				if(verbose_flag) printf("a_pos = %d; b_pos = %d; c_pos = %d\n", a_pos, b_pos, c_pos);
				for(j = a_pos; j < (i_n + ((i * s_array[0].length) << 1)); j++){
					if((a_pos < j_n) && ((b_pos >= c_pos) || (array[a_pos] <= array[b_pos]))){
						if(verbose_flag) printf("element %d of `array` is less than or equal to element %d of `array`\n", a_pos, b_pos);
						temp_array[j] = array[a_pos];
						a_pos++;
						if(verbose_flag) printf("temp_array[%d] = %d\n", j, temp_array[j]);
					}else{
						if(verbose_flag) printf("element %d of `array` is less than or equal to element %d of `array`\n", b_pos, a_pos);
						temp_array[j] = array[b_pos];
						b_pos++;
						if(verbose_flag) printf("temp_array[%d] = %d\n", j, temp_array[j]);
					}
				}
			}
			t = !t;
		}
	}
	if(verbose_flag) printf("t = %d\n", t);
	if(t){
		if(verbose_flag) printf("copying `temp_array` to `array`\n", t);
		for(i = 0; i < array_length; i++){
			array[i] = temp_array[i];
		}
	}else{
		if(verbose_flag) printf("since t is 0, do nothing\n", t);
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

	// once all threads terminate, we free the `threads` array
	free(threads);

	// and then we perform the merge
	merge(sub_arrs, 0, MAX_THREADS, arr, 0, num_elements);

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
