/*
 * tasks.h
 *
 *  Created on: Oct 22, 2024
 *      Author: athul-muralidhar
 */

#ifndef TASKS_H_
#define TASKS_H_

#include "kernel.h"

uint32_t Task0_profiler, Task1_profiler, Task2_profiler, PeriodicTask1_profiler;

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
//		os_thread_yield();
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

void task3() {
	PeriodicTask1_profiler++;
}

#endif /* TASKS_H_ */
