/*
 * Simulator.h
 *
 *  Created on: Mar 10, 2016
 *      Author: vens
 */

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

#include <fstream>
#include <list>
#include "libjson/json.h"
#include "libwavedrom/wavedrom.hpp"

class SimTask;

class Simulator {
public:
	Simulator();
	virtual ~Simulator();

	void MakeTaskList(json_value * array);

	// return true if all the tasks in the task list are finished
	bool IsDone();
	// run one system tick on the scheduler
	void RunTick();

	// export the json waveform file
	void ExportWaveform(std::ofstream& out);

private:
	std::list<SimTask*> not_arrived_list;
	std::list<SimTask*> ready_list;
	std::list<SimTask*> blocked_list;
	std::list<SimTask*> finished_list;
	SimTask* running_task;

	unsigned int num_tasks;
	unsigned int remaining_tasks;

	unsigned long sys_time;

	wavedrom::Wavedrom wave_root;
	wavedrom::Signal * wave_running;
};

#endif /* SIMULATOR_H_ */
