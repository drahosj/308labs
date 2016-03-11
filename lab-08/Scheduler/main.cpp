/*
 * main.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: vens
 */

#include <iostream>
#include <fstream>
#include <string.h>
#include <list>

#include "Simulator.h"
#include "SimTask.h"
#include "libmodule/Module.h"

#include <stdio.h>
#include "libjson/json.h"

using namespace std;

/*
static void print_depth_shift(int depth)
{
        int j;
        for (j=0; j < depth; j++) {
                printf(" ");
        }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.object.length;
        for (x = 0; x < length; x++) {
                print_depth_shift(depth);
                printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
                process_value(value->u.object.values[x].value, depth+1);
        }
}

static void process_array(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.array.length;
        printf("array\n");
        for (x = 0; x < length; x++) {
                process_value(value->u.array.values[x], depth);
        }
}

static void process_value(json_value* value, int depth)
{
        int j;
        if (value == NULL) {
                return;
        }
        if (value->type != json_object) {
                print_depth_shift(depth);
        }
        switch (value->type) {
                case json_none:
                        printf("none\n");
                        break;
                case json_object:
                        process_object(value, depth+1);
                        break;
                case json_array:
                        process_array(value, depth+1);
                        break;
                case json_integer:
                        printf("int: %10" PRId64 "\n", value->u.integer);
                        break;
                case json_double:
                        printf("double: %f\n", value->u.dbl);
                        break;
                case json_string:
                        printf("string: %s\n", value->u.string.ptr);
                        break;
                case json_boolean:
                        printf("bool: %d\n", value->u.boolean);
                        break;
        }
}*/

/*void make_task_list(json_value* array, list<SimTask> & list)
{
	int length, x;
	length = array->u.array.length;
	for(x=0;x<length;x++)
	{
		list.emplace_back(array->u.array.values[x]);
	}
	list.sort();
}*/

int main(int argc, const char * argv[])
{
	Simulator simulator;

	module::Module mod("../sched-fcfs.mod");
	ifstream task_list_file("task_list.json");
	task_list_file.seekg(0, task_list_file.end);
	int length = task_list_file.tellg();
	task_list_file.seekg(0, task_list_file.beg);

	char * buf = new char [length];

	task_list_file.read(buf, length);

	//const char* buf = "{\"name\": \"task name\",\"priority\": 1,\"arrive-time\": 100,\"run-times\": [ 1,4,3,5],\"block-times\": [2,3,1],\"deadline\":  125}";
	//size_t len = strlen(buf);

	json_value * task_list = json_parse(buf, length);

	//int length, x;
	length = task_list->u.object.length;
	cout << "length=" << length << endl;
	cout << "name=" << task_list->u.object.values[0].name;
	//make_task_list(task_list->u.object.values[0].value, list);

	simulator.MakeTaskList(task_list->u.object.values[0].value);

	for(int i=0;i<5;i++)
	//while(!simulator.IsDone())
	{
		simulator.RunTick();
	}

	ofstream out("wave.json");
	simulator.ExportWaveform(out);

	//process_value(task, 0);

	//SimTask t(task);

	std::cout << "Hello World" << std::endl;
	return 0;
}

