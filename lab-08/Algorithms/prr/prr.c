#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "task.h"
#include "scheduler_algorithm.h"

#define SCHEDULER_NAME "PRR"

const struct scheduler_operations sops;

int verbose_flag = 1;

static int handle;

/* 4 heads because there are 4 queues because 4 priorities */
struct task * heads[4];

int init_module(void)
{
	handle = register_scheduler(SCHEDULER_NAME, &sops);
	if(handle < 0){
		eprintf("Failed registering scheduler PRR with error %d\n", handle);
		return handle;
	}
	else{
		dprintf("Registered scheduler PRR\n");
	}
	return 0;
}

void cleanup_module(void)
{
	int rv = unregister_scheduler(handle);
	if(rv < 0) eprintf("Error in unregister_scheduler: %d\n", rv);
}

struct task* prr_enqueue(struct task* r, struct task* t)
{
	if (heads[t->task_info->priority] == NULL) {
		heads[t->task_info->priority] = t;
		t->next = heads[t->task_info->priority];
		t->prev = heads[t->task_info->priority];
	} else {
		heads[t->task_info->priority]->prev->next = t;
		t->prev = heads[t->task_info->priority]->prev;
		heads[t->task_info->priority]->prev = t;
		t->next = heads[t->task_info->priority];
	}
	return heads[t->task_info->priority];
}

struct task* prr_dequeue(struct task* r)
{
	if (r->next == r) {
		heads[r->task_info->priority] = NULL;
	} else {
		r->prev->next = r->next;
		r->next->prev = r->prev;
		heads[r->task_info->priority] = r->next;
	}
	return heads[r->task_info->priority];
}

struct task* prr_periodic_timer(struct task* r)
{
	struct task * head = NULL;
	if (heads[3]) {
		head = heads[3]->next;
	} else if (heads[2]) {
		head = heads[2]->next;
	} else if (heads[1]) {
		head = heads[1]->next;
	} else if (heads[0]) {
		head = heads[0]->next;
	}
	return head;
}

const struct scheduler_operations sops =
{
	.task_enqueue = prr_enqueue,
	.task_dequeue = prr_dequeue,
	.periodic_timer = prr_periodic_timer,
	.period = 1
};
