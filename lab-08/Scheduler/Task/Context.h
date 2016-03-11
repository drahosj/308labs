/*
 * Context.h
 *
 *  Created on: Mar 10, 2016
 *      Author: vens
 */

#ifndef TASK_CONTEXT_H_
#define TASK_CONTEXT_H_

class Context {
public:
	Context() {};
	virtual ~Context() {};

	virtual void SwapIn(unsigned long sys_time) {};
	virtual void SwapOut(unsigned long sys_time) {};
};

#endif /* TASK_CONTEXT_H_ */
