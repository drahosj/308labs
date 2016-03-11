/*
 * SchedulerAlgorithm.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

#include "SchedulerAlgorithm.h"

SchedulerAlgorithm::SchedulerAlgorithm(std::string _name, const struct scheduler_operations * _sops)
	: name(_name), sops(_sops), ticks(0)
{

}

SchedulerAlgorithm::~SchedulerAlgorithm() {
	// TODO Auto-generated destructor stub
}

struct task * SchedulerAlgorithm::EnqueueTask(struct task * running_task, struct task * new_task)
{
	struct task * t = this->sops->task_enqueue(running_task, new_task);
	if(t && t != running_task)
		this->ticks = 0;
	return t;
}

struct task * SchedulerAlgorithm::DequeueTask(struct task * running_task)
{
	struct task * t = this->sops->task_dequeue(running_task);
	if(t)
		this->ticks = 0;
	return t;
}

struct task * SchedulerAlgorithm::OnTick(struct task * running_task)
{
	this->ticks ++;
	if(this->sops->period == this->ticks)
	{
		return this->sops->periodic_timer(running_task, this->ticks);
	}
	return running_task;
}
