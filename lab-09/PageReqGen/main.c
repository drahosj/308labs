
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "sim_page_req.h"

// 0 is random; 1 is sequential; 2 is temporal
int mem_access = 0;
int num_tasks = 100;
int seed = 5;

/**
 * Recognized arguments:
 * - `-n`: number of page requests to generate
 * - `-s`: sets the seed
 * - `--temporal': temporal locality, i.e., there is a 90% chance that the next
 *   		   memory request will be from the same page as one of the last
 *   		   5 page requests
 * - `--sequential`: sequential memory access, i.e., page 1, page 2, page 3, ...
 * - `--random`: random memory access
 */
struct option long_options[] = {
	// these options set a flag
	{"temporal", no_argument, &mem_access, 2},
	{"sequential", no_argument, &mem_access, 1},
	{"random", no_argument, &mem_access, 0},
	// these options do not set a flag
	{"num-tasks", required_argument, 0, 'n'},
	{"seed", required_argument, 0, 's'},
	{0, 0, 0, 0}
};

int main(int argc, char * argv[])
{
	int c;
	int opt_idx;
	int i;
	int j;
	int last_arrive_time;
	int last_pages_filled = 0;
	unsigned int last_page = 0;
	int last_pages[5] = {-1, -1, -1, -1, -1};
	struct sim_page_req *page_req = NULL;

	c = getopt_long(argc, argv, "n:s:?", long_options, &opt_idx);
	while(c != -1){
		switch(c){
			case 0: if(long_options[opt_idx].flag != 0){
					printf("Option %s was called, setting value to %d\n", long_options[opt_idx].name, long_options[opt_idx].val);
					break;
				}
				if(optarg){
					
				}
				break;
			case 'n': // set the number of tasks to create
				  num_tasks = atoi(optarg);
				  printf("Number of tasks to generate: %d\n", num_tasks);
				  break;
			case 's': // set the seed for the random number generator
				  seed = atoi(optarg);
				  printf("New seed set to: %d\n", seed);
				  break;
		}
		c = getopt_long(argc, argv, "m:M:n:s:?", long_options, &opt_idx);
	}

	// set the seed for the random number generator
	srand(seed);

	// initialize the json task list
	init_json_tasklist();

	// populate the task list
	for(i = 0; i < num_tasks; i++){
		// first allocate memory for the task
		page_req = calloc(1, sizeof(struct sim_page_req));
		printf("generated a task\n");

		// set arrival time
		if(i == 0){
			page_req->arrive_time = (unsigned long)(rand() % 5);
		}else{
			page_req->arrive_time = last_arrive_time + (rand() % 15);
		}
		last_arrive_time = page_req->arrive_time;
		printf("\tArrival time: %lu\n", page_req->arrive_time);

		// set page to be requested
		switch(mem_access){

		// if random
		case 0:	page_req->page = rand() % 32;
			break;

		// if sequential
		case 1:	page_req->page = last_page++ % 32;
			break;

		// if temporal
		case 2:	if(last_pages_filled < 5 || (rand() % 10) > 8){
				page_req->page = (unsigned long)(rand() % 32);
				while(j < 5){
					while((int)page_req->page == last_pages[j]){
						page_req->page = (unsigned long)(rand() % 32);
					}
					j++;
				}
				if(last_pages_filled < 5) last_pages_filled++;
			}else{
				page_req->page = (unsigned long)last_pages[rand() % 5];
			}
			// add current page to the list of pages
			for(j = 0; j < 4; j++){
				last_pages[j] = last_pages[j + 1];
				printf("\t\tlast_pages[%d] = %d\n", j, last_pages[j]);
			}
			last_pages[j] = (int)page_req->page;
			printf("\t\tlast_pages[%d] = %d\n", j, last_pages[j]);
			break;
		}
		printf("\tpage: %lu\n", page_req->page);
		
		// add this task to the json
		add_sim_task_to_json(page_req);
		// "let go" of the task
		page_req = NULL;
	}

	FILE* fp = fopen("out.json", "w");
	export_json_tasklist(fp);
	fclose(fp);
	return 0;
}
