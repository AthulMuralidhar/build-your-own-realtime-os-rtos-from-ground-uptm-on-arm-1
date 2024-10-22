/*
 * tasks.h
 *
 *  Created on: Oct 22, 2024
 *      Author: athul-muralidhar
 */

#ifndef TASKS_H_
#define TASKS_H_

#include "kernel.h"

uint32_t Task0_profiler, Task1_profiler, Task2_profiler;
void stop_motor() {
	ITMPrint("stopping motor...\n");
}
void run_motor() {
	ITMPrint("running motor...\n");
}
void open_valve() {
	ITMPrint("open valve ...\n");
}
void close_valve() {
	ITMPrint("close valve...\n");
}

void task0() {
	while (1) {
		Task0_profiler++;
		// this OS thread function yields the main execution thread
		// after one execution of this while loop whereas the other tasks
		/// continue on executing inside their while loops
		//  this can be seen by using the expressions on the various
		// profilers available
		os_thread_yield();
	}
}

void task1() {
	while (1) {
		Task1_profiler++;
//		stop_motor();

	}
}

void task2() {
	while (1) {
		Task2_profiler++;
//		open_valve();
	}
}

#endif /* TASKS_H_ */
