/*
 * module.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

#include "Module.h"

#include <string>
#include <iostream>

#include <dlfcn.h>


namespace module {

Module::Module(const std::string name) {
	char * error;
	int rv;
	int (*init_function)();

	this->module_handle = dlopen(/*name.c_str()*/"./sched-fcfs.mod", RTLD_NOW);
	if(!this->module_handle){
		// TODO throw exception;
		std::cerr << dlerror();
		return;
	}

	init_function = (int (*)())dlsym(this->module_handle, "init_module");
	if((error = dlerror()) != NULL){
		// TODO throw exception
		std::cerr << "Error 2";
		return;
	}

	this->module_name = name;

	rv = init_function();
	if(rv){
		// TODO throw exception
		std::cerr << "Error 3";
		return;
	}

	return;
}

Module::~Module() {
	dlclose(this->module_handle);
}

} /* namespace module */
