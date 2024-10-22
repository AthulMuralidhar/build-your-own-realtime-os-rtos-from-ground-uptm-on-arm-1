/*
 * DEVELOPER NOTES
 *
 *
 *
 * */

#include <stdint.h>
#include "./chip-headers/stm32f407xx.h"
#include "printer.h"
#include "kernel.h"
#include "tasks.h"

#define QUANTUM		10

int main(void) {
	os_kernel_add_threads(&task0, &task1, &task2);

	// set time quantum
	os_kernel_launch(QUANTUM);

}

