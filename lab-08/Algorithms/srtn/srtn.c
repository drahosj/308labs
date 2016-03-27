#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "task.h"
#include "scheduler_algorithm.h"

#define SCHEDULER_NAME "SRTN"

struct scheduler_data {
	struct scheduler_data * next;
	unsigned long last_run_time;
	unsigned long guess;
};

struct scheduler_data * sdata_head;

const struct scheduler_operations sops;

int verbose_flag = 1;

static int handle;

static struct task *head = NULL;

int init_module(void)
{
	handle = register_scheduler(SCHEDULER_NAME, &sops);
	if(handle < 0){
		eprintf("Failed registering scheduler SRTN with error %d\n", handle);
		return handle;
	}
	else{
		dprintf("Registered scheduler SRTN\n");
	}
	return 0;
}

void cleanup_module(void)
{
	int rv = unregister_scheduler(handle);
	if(rv < 0) eprintf("Error in unregister_scheduler: %d\n", rv);

	/* Clean up all scheduler datas */
	struct scheduler_data * sdata_next = NULL;
	while (sdata_head) {
		sdata_next = sdata_head->next;
		// Something actually frees all of this at the end, and
		// freeing it now breaks things later.
		// free(sdata_head);
		sdata_head = sdata_next;
	} 
}

static struct task * find_srt()
{
	struct task * cur = head;
	struct task * least = cur;

	if (cur == NULL) {
		return NULL;
	}
	do {
		if (((struct scheduler_data *) cur->scheduler_data)->guess < ((struct scheduler_data *) least->scheduler_data)->guess) {
			least = cur;
		}
		cur = cur->next;
	} while (cur != head);

	head = least;
	return head;
}

struct task* srtn_enqueue(struct task* r, struct task* t)
{
	/* Create scheduler data if needed, and add it to the linked
	 * list of scheduler datas for cleanup
	 */
	if (((struct scheduler_data *) t->scheduler_data) == NULL) {
		t->scheduler_data = malloc(sizeof(struct scheduler_data));
		if (((struct scheduler_data *) t->scheduler_data) == NULL) {
			perror("malloc");
			abort();
		}

		((struct scheduler_data *) t->scheduler_data)->next = NULL;
		((struct scheduler_data *) t->scheduler_data)->guess = 2 * (4 - t->task_info->priority);
		((struct scheduler_data *) t->scheduler_data)->last_run_time = 0;

		if (sdata_head == NULL) {
			sdata_head = ((struct scheduler_data *) t->scheduler_data);
		} else {
			struct scheduler_data * sdata_next = sdata_head;
			while (sdata_next->next) {
				sdata_next = sdata_next->next;
			}
			sdata_next->next = ((struct scheduler_data *) t->scheduler_data);
		}
	} else {
		((struct scheduler_data *) t->scheduler_data)->guess = (2 * (((struct scheduler_data *) t->scheduler_data)->guess) + 
				8 * (t->task_info->run_time - ((struct scheduler_data *) t->scheduler_data)->last_run_time))/10;
		((struct scheduler_data *) t->scheduler_data)->last_run_time = t->task_info->run_time;
	}

	if (head == NULL) {
		head = t;
		t->next = head;
		t->prev = head;
	} else {
		head->prev->next = t;
		t->prev = head->prev;
		head->prev = t;
		t->next = head;
	}

	/* Assess all tasks and return the shortest remaining time */
	return find_srt();
}

struct task* srtn_dequeue(struct task* r)
{
	if (r->next == r) {
		head = NULL;
	} else {
		r->prev->next = r->next;
		r->next->prev = r->prev;
		head = r->next;
	}
	return find_srt();
}

const struct scheduler_operations sops =
{
	.task_enqueue = srtn_enqueue,
	.task_dequeue = srtn_dequeue,
};
