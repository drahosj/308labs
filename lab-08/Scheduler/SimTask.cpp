/*
 * SimTask.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: vens
 */

#include <iostream>
#include "Scheduler/Scheduler.h"
#include "SimTask.h"
#include "Task/Task.h"

enum TaskJsonField
{
	NAME,
	PRIORITY,
	ARRIVE_TIME,
	RUN_TIMES,
	BLOCK_TIMES,
	DEADLINE,
	UNKNOWN
};

TaskJsonField NameToField(std::string const& str)
{
	if(str == "name") return NAME;
	if(str == "priority") return PRIORITY;
	if(str == "arrive-time") return ARRIVE_TIME;
	if(str == "run-times") return RUN_TIMES;
	if(str == "block-times") return BLOCK_TIMES;
	if(str == "deadline") return DEADLINE;
	else return UNKNOWN;
}


SimTask::~SimTask() {
	// TODO Auto-generated destructor stub
}

SimTask::SimTask(json_value * task, wavedrom::Group * wave_grp)
	: state(SimTask::INIT), last_state(SimTask::INIT), times_index(0), remove_flag(false)
{
	int length, array_len;
	length = task->u.object.length;
	for(int i=0; i<length; i++)
	{
		json_value *value = task->u.object.values[i].value;
		std::string val_name = task->u.object.values[i].name;
		std::cout << "processing value: " << val_name << std::endl;
		switch(NameToField(val_name))
		{
		case NAME:
			this->name = value->u.string.ptr;
			this->SetName(this->name);
			break;
		case PRIORITY:
			this->priority = value->u.integer;
			this->SetPriority(this->priority);
			break;
		case ARRIVE_TIME:
			this->next_arrival_time = value->u.integer;
			break;
		case RUN_TIMES:
			array_len = value->u.array.length;
			for(int j=0; j<array_len; j++)
			{
				this->times[j*2] = value->u.array.values[j]->u.integer;
			}
			this->times[array_len * 2 - 1] = 0;
			this->num_runs = array_len;
			break;
		case BLOCK_TIMES:
			array_len = value->u.array.length;
			for(int j=0; j<array_len; j++)
			{
				this->times[j*2 + 1] = value->u.array.values[j]->u.integer;
			}
			break;
		case DEADLINE:
			this->deadline = value->u.integer;
			this->SetDeadline(this->deadline);
			break;
		default:
			std::cerr<< "Unrecogninzed Json Field: " << val_name << std::endl;
		}
	}

	this->wave = wave_grp->AddSignal(this->name.c_str());

	this->task = 0;

//	std::cout << "name: " << this->name << std::endl;
//	std::cout << "prior: " << this->priority << std::endl;
//	std::cout << "arrive: " << this->next_arrival_time << std::endl;
//	std::cout << "times: ";
//	for(int i=0;i<this->num_runs * 2;i++) std::cout << this->times[i] << ", ";
//	std::cout << std::endl;
//	std::cout << "deadline: " << this->deadline << std::endl;
}


bool SimTask::operator< (const SimTask& param)
{
	return this->next_arrival_time < param.next_arrival_time;
}

unsigned long SimTask::GetArrivalTime()
{
	return this->next_arrival_time;
}

/*
// this is called when the task is moved to the ready queue
void SimTask::OnReady(unsigned long sys_time)
{
	this->wave->AddNode(wavedrom::NODE::Z);
}

void SimTask::OnBlock(unsigned long sys_time)
{
	this->wave->AddNode(wavedrom::NODE::DOWN);
}
*/

void SimTask::OnRunTick(unsigned long sys_time)
{
	this->wave->AddNode(wavedrom::NODE::HIGH);
}

bool SimTask::IsFinished()
{
	return this->state == SimTask::FINISHED;
}

//
void SimTask::OnStartTick(unsigned long sys_time)
{

	if(this->state == SimTask::INIT)
	{
		this->state = SimTask::NOT_ARRIVED;
	}

	if(this->next_arrival_time == sys_time)
	{
		switch(this->state)
		{
		case SimTask::NOT_ARRIVED:
			this->last_state = this->state;
			this->state = SimTask::READY;
			this->OnArrive(sys_time);
			//this->task = new Task(this->name, this->priority);
			Scheduler::AddTask(this, sys_time);
			break;
		case SimTask::RUNNING:
			break;
		case SimTask::READY:
			break;
		case SimTask::BLOCKED:
			this->last_state = this->state;
			this->state = SimTask::READY;
			Scheduler::AddTask(this, sys_time);
			//this->task->MoveReady();
			break;
		case SimTask::FINISHED:
			break;

		}
	}
}

void SimTask::SwapIn(unsigned long sys_time)
{
	//this->last_state = this->state;
	this->state = SimTask::RUNNING;
}

void SimTask::SwapOut(unsigned long sys_time)
{
	this->state = SimTask::READY;
}

void SimTask::OnSysTick(unsigned long sys_time)
{
	if(this->state == this->last_state)
	{
		this->wave->ContinueNode();
	}
	else
	{
		switch(this->state)
		{
		case SimTask::NOT_ARRIVED:
			this->wave->AddNode(wavedrom::NODE::LOW);
			break;
		case SimTask::READY:
			this->wave->AddNode(wavedrom::NODE::X);
			break;
		case SimTask::RUNNING:
			this->wave->AddNode(wavedrom::NODE::WHITE);
			break;
		case SimTask::BLOCKED:
			this->wave->AddNode(wavedrom::NODE::RED);
			break;
		case SimTask::FINISHED:
			this->wave->AddNode(wavedrom::NODE::LOW);
			break;
		}
		this->last_state = this->state;
	}

	this->remove_flag = false;
	if(this->state == SimTask::RUNNING)
	{
		this->times[this->times_index]--;
		if(this->times[this->times_index] == 0)
		{
			this->remove_flag = true;
			this->times_index++;
			if(this->times[this->times_index] > 0)
			{
				this->last_state = this->state;
				this->state = SimTask::BLOCKED;
				this->next_arrival_time = sys_time + this->times[this->times_index];
				this->times[this->times_index] = 0;
				this->times_index ++;
			}
			else
			{
				this->last_state = this->state;
				this->state = SimTask::FINISHED;
			}
		}
	}
}

void SimTask::OnEndTick(unsigned long sys_time)
{
	if(this->remove_flag)
	{
		SimTask::State tmp = this->state;
		Scheduler::RemoveTask(sys_time);
		this->state = tmp;
	}
}
