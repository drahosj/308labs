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

/*
std::list<SimTask*> not_arrived_list;
	std::list<SimTask*> ready_list;
	std::list<SimTask*> blocked_list;
	std::list<SimTask*> finished_list;
*/
Simulator::Simulator()
{
	// TODO Auto-generated constructor stub
	this->wave_root.AddSignal("Running");
}

Simulator::~Simulator() {
	// TODO Auto-generated destructor stub
}

void Simulator::MakeTaskList(json_value * array)
{
	this->num_tasks = array->u.array.length;
	for(int x=0;x<this->num_tasks;x++)
	{
		SimTask * task = new SimTask(array->u.array.values[x], &this->wave_root);
		this->not_arrived_list.push_back(task);
	}
	this->remaining_tasks = this->num_tasks;

	this->not_arrived_list.sort();

//	for(std::list<SimTask*>::iterator it = not_arrived_list.begin(); it != not_arrived_list.end(); ++it)
//	{
//		std::cout << (*it)->name << std::endl;
//	}
}

bool Simulator::IsDone()
{
	return this->remaining_tasks == 0;
}

void Simulator::RunTick()
{
	std::list<SimTask*>::iterator it;


	// for each task in system
	// call OnTick(sys_time)
		// Task handels own lifecycle by calling scheduler directly
		// returns its state (not_arrived, ready, running, blocked, finished);












	/*for(std::list<SimTask*>::iterator it = not_arrived_list.begin(); it != not_arrived_list.end(); ++it)
	{
		std::cout << (*it)->name << std::endl;
	}*/

	// for each task in the not arrived list, see if it has arrived and if so move it to ready
	for(it=this->not_arrived_list.begin(); it != this->not_arrived_list.end(); )
	{
		SimTask* this_task = *it;
		if((*it)->GetArrivalTime() == this->sys_time)
		{
		//	this->ready_list.splice(this->ready_list.begin(), this->not_arrived_list, it);
			it = this->not_arrived_list.erase(it);
			this_task->OnReady(this->sys_time);
			this->ready_list.push_back(this_task);
			//++it;
		}
		else
		{
			++it;
		}
	}

	// for each task in the blocked list, see if it is done being blocked
	for(it=this->blocked_list.begin(); it != this->blocked_list.end();)
	{
		SimTask* this_task = *it;
		if(this_task->GetArrivalTime() == this->sys_time)
		{
			it = this->blocked_list.erase(it);
			this_task->OnReady(this->sys_time);
			this->ready_list.push_back(this_task);
		}
		else
		{
			++it;
		}
	}

	// run the sys tick on the scheduler
//	Scheduler::OnSysTick(sys_time);

	if(this->running_task)
	{
		this->running_task->OnRunTick(sys_time);
	}


	// advance the sys time to show the end of this tick
	this->sys_time ++;
}

void Simulator::ExportWaveform(std::ofstream& out)
{
	char * buf = this->wave_root.Export();
	out << std::string(buf);
}
