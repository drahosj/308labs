
#include <stdio.h>
#include <getopt.h>
#include "sim_task.h"

int interactive = 0;
unsigned long minimum = 1;
unsigned long maximum = 5;
int num_tasks = 500;
unsigned long deadline = 20;

/**
 * Recognized arguments:
 * - `-m`: the minimum amount of time that a task will run
 * - `-M`: the maximum amount of time that a task will run
 * - `-i': interactive workload
 * - `-b`: batch workload
 * - `-n`: number of tasks to generate
 * - `-d`: deadline for the tasks
 */
static struct option long_options[] = {
	{"min", required_argument, 0, 'm'},
	{"max", required_argument, 0, 'M'},
	{"interactive", no_argument, &interactive, 1},
	{"batch", no_argument, &interactive, 0},
	{"num-tasks", required_argument, 0, 'n'},
	{"deadline", required_argument, 0, 'd'}
}

int main(int argc, const char * argv[])
{
	int c;
	int opt_idx;
	int i;

	c = getopt_long(argc, argv, "m:M:ibn:d:?", long_options, &opt_idx);
	while(c != -1){
		switch(c){
			case 0: if(long_options[opt_idx].flag != 0) break;
				// print something?
				if(optarg){
					
				}
				break;
			case 'm': // set the minumum time
				  minimum = atoi(optarg);
				  break;
			case 'M': // set the maximum time
				  maximum = atoi(optarg);
				  break;
			case 'i': // print something?
				  break;
			case 'b': // print something?
				  break;
			case 'n': // set the number of tasks to create
				  num_tasks = atoi(optarg);
				  break;
			case 'd': // set the deadline for the 
				  break;
		}
		c = getopt_long(argc, argv, "m:M:ibn:d:?", long_options, &opt_idx);
	}

	//struct sim_task t1 = {"A", 2, 14, {1,2,3,4,5,0}, 100};
	//struct sim_task t2 = {"B", 4, 16, {3,2,1}, 20};

	init_json_tasklist();
	add_sim_task_to_json(&t1);
	add_sim_task_to_json(&t2);
	FILE* fp = fopen("out.json", "w");
	export_json_tasklist(fp);
	fclose(fp);
	return 0;
}
