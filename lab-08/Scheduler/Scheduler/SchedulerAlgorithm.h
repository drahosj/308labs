/*
 * SchedulerAlgorithm.h
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

#ifndef SCHEDULERALGORITHM_H_
#define SCHEDULERALGORITHM_H_

#include <string>

#include "Scheduler.h"

#include "../scheduler_algorithm.h"



class SchedulerAlgorithm {
	friend class Scheduler;

public:
	SchedulerAlgorithm(std::string name, const struct scheduler_operations * sops);
	virtual ~SchedulerAlgorithm();

	struct task * EnqueueTask(struct task * running_task, struct task * new_task);
	struct task * DequeueTask(struct task * running_task);
	struct task * OnTick(struct task * running_task);

private:
	const std::string name;
	const struct scheduler_operations * sops;
	unsigned long ticks;

	int handle = 0;
};


#endif /* SCHEDULERALGORITHM_H_ */
