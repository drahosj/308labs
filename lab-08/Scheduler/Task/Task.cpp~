/*
 * Task.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

//#include "Kernel.h"
#include "Task.h"

Task::Task(std::string name, unsigned char priority) {
	std::size_t len = name.copy(this->task_info.name, 128, 0);
	this->task_info.name[len] = '\0';
	this->task_info.priority = priority;
//	this->task_info.time_arive = Kernel::GetSysTime();
	this->task_info.block_time = 0;
	this->task_info.run_time = 0;
	this->task_info.state = task_info::TASK_READY;
	this->task_info.parrent = (void*) this;

	this->task.task_info = &this->task_info;
	this->task.next = 0;
	this->task.prev = 0;
	this->task.scheduler_data = 0;
}

Task::~Task() {
	// TODO Auto-generated destructor stub
}

void Task::MoveBlocked()
{
	//this->task.task_info->block_time ++;
}
