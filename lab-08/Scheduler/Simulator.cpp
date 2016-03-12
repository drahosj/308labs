/*
 * Simulator.cpp
 *
 *  Created on: Mar 10, 2016
 *      Author: vens
 */

#include <iostream>
#include "SimTask.h"
#include "Scheduler/Scheduler.h"
#include "Simulator.h"

Simulator::Simulator()
	: remaining_tasks(0)
{
	// TODO Auto-generated constructor stub
	this->wave_running = this->wave_root.AddSignal("Running");
	this->running_task = 0;
	this->num_tasks = 0;
	this->sys_time = 0; 
}

Simulator::~Simulator() {
	// TODO Auto-generated destructor stub
	for(auto it=this->sim_task_list.begin(); it != this->sim_task_list.end();++it)
	{
		delete *it;
	}

	for(auto it=this->finished_list.begin(); it != this->finished_list.end();++it)
	{
		delete *it;
	}
}

void Simulator::MakeTaskList(json_value * array)
{
	this->num_tasks = array->u.array.length;
	for(int x=0;x<this->num_tasks;x++)
	{
		SimTask * task = new SimTask(array->u.array.values[x], &this->wave_root);
		this->sim_task_list.push_back(task);
	}
	this->remaining_tasks = this->num_tasks;

	this->sim_task_list.sort();

}

bool Simulator::IsDone()
{
	return this->remaining_tasks == 0;
}

void Simulator::RunTick()
{
	std::list<SimTask*>::iterator it;
	static SimTask* running_task = 0;
	SimTask* last_running_task = running_task;
	running_task = 0;

	for(it=this->sim_task_list.begin(); it != this->sim_task_list.end();++it)
	{
		SimTask* this_task = *it;
		this_task->OnStartTick(this->sys_time);

	}


	Scheduler::OnSysTick(this->sys_time);

	// advance the sys time to show the end of this tick
	this->sys_time ++;

	for(it=this->sim_task_list.begin(); it != this->sim_task_list.end();++it)
	{
		SimTask* this_task = *it;
		if(this_task->IsRunning())
		{
			running_task = this_task;
		}
		this_task->OnSysTick(this->sys_time);
	}

	for(it=this->finished_list.begin(); it != this->finished_list.end(); ++it)
	{
		SimTask* this_task = *it;
		this_task->OnSysTick(sys_time);
	}

	for(it=this->sim_task_list.begin(); it != this->sim_task_list.end();)
	{
		SimTask* this_task = *it;
		this_task->OnEndTick(this->sys_time);
		if(this_task->IsFinished())
		{
			it = this->sim_task_list.erase(it);
			this->finished_list.push_back(this_task);
			this->remaining_tasks --;
		}
		else
		{
			++it;
		}
	}

	if(running_task == 0)
	{
		this->wave_running->AddNode(wavedrom::NODE::Z);
	}
	else if(running_task == last_running_task)
	{
		this->wave_running->ContinueNode();
	}
	else if(running_task)
	{
		this->wave_running->AddNode(wavedrom::NODE::WHITE, running_task->GetName().c_str());
	}
}

void Simulator::ExportWaveform(std::ofstream& out)
{
	char * buf = this->wave_root.Export();
	out << std::string(buf);
	delete[] buf;
}
