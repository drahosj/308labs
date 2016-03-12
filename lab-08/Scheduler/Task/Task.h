/*
 * Task.h
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

#ifndef TASK_H_
#define TASK_H_

#include <string>
#include "Context.h"
#include "../task.h"

class Task : public Context{
	friend class Scheduler;
public:
	//Task(std::string name, unsigned char priority);
	Task();
	virtual ~Task();

	void OnArrive(unsigned long sys_time);

	void SetPriority(int p);

	void SetName(std::string name);

	void SetDeadline(unsigned long t);

	// called when this task blocks
	void MoveBlocked();

	// called when this task is ready to run
	void MoveReady();

	// called when this task is given the cpu
	void MoveRunning();

	// called on every system tick while the task is running
	void RunTick();

private:
	struct task task;
	struct task_info task_info;
	//Context * context;

	/// The system idle task.  This task is never added to the scheduler queue,
	/// but will be run any time the scheduler has nothing else to run
	//static Task idle_task;
};

#endif /* TASK_H_ */
