
#include <stdlib.h>
#include <stdio.h>

#include "libjson/json.h"
#include "sim_page_req.h"

json_value* list;

const json_serialize_opts json_opts =
{
	.mode = json_serialize_mode_multiline,
	.opts = 0,
	.indent_size = 2
};

void init_json_tasklist(void)
{
	list = json_array_new(0);
}

void add_sim_task_to_json(struct sim_page_req * task)
{
	json_value* t = json_object_new(0);
	json_object_push(t, "arrive-time", json_integer_new(task->arrive_time));
	json_object_push(t, "page", json_integer_new(task->page));
	json_array_push(list, t);
}

void export_json_tasklist(FILE* fp)
{
	json_value * root = json_object_new(0);
	json_object_push(root, "tasks", list);
	size_t len = json_measure_ex(root, json_opts);
	char * buf = malloc(len);
	json_serialize_ex(buf, root, json_opts);
	fprintf(fp, "%s\n", buf);
	//fwrite(buf, len - 1, 1, fp);
}
