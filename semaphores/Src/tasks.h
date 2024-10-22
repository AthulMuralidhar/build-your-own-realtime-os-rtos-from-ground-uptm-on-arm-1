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
uint32_t semaphore1, semaphore2;


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
		os_semaphore_wait(&semaphore1);
		Task1_profiler++;
		run_motor();
		os_semaphore_set(&semaphore2);

	}
}

void task2() {
	while (1) {
		os_semaphore_wait(&semaphore2);
		Task2_profiler++;
		open_valve();
		os_semaphore_set(&semaphore1);
	}
}

void task3() {
	PeriodicTask1_profiler++;
}

#endif /* TASKS_H_ */
