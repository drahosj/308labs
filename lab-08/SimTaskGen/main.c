
#include <stdio.h>

#include "sim_task.h"

int main(int argc, const char * argv[])
{
	struct sim_task t1 = {"A", 2, 14, {1,2,3,4,5,0}, 100};
	struct sim_task t2 = {"B", 4, 16, {3,2,1}, 20};

	init_json_tasklist();
	add_sim_task_to_json(&t1);
	add_sim_task_to_json(&t2);

	FILE* fp = fopen("out.json", "w");
	export_json_tasklist(fp);
	fclose(fp);
	return 0;
}
