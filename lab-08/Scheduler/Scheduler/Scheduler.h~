/*
 * Scheduler.h
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <vector>


class SchedulerAlgorithm;
class Task;

class Scheduler {
	//friend class SchedulerAlgorithm;

public:

	static int RegisterAlgorithm(SchedulerAlgorithm *algorithm);
	static int UnregisterAlgorithm(int handle);

	static void AddTask(Task* task, unsigned long sys_time);
	static void RemoveTask(unsigned long sys_time);
	static void OnSysTick(unsigned long sys_time);

private:
	static SchedulerAlgorithm* algorithm;

	static Task* running_task;
};

#endif /* SCHEDULER_H_ */
