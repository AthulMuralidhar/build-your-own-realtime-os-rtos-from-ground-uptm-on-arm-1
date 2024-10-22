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

#define QUANTUM		2   // in ms
#define TIM2_SR_UIF		(1<<0)

uint32_t Periodic2_Profiller;
//uint32_t semaphore1, semaphore2;


int main(void) {
	// initialize hardware timer
	TIM2_1hz_interrupt_init();

	// Initialize semaphores
	os_semaphore_init(&semaphore1, 1);
	os_semaphore_init(&semaphore2, 0);

	os_kernel_add_threads(&task0, &task1, &task2);

	// set time quantum
	os_kernel_launch(QUANTUM);

}

void TIM2_IRQHandler() {
	// clear the update interrupt flag
	TIM2->SR &= ~TIM2_SR_UIF;
	// run
	Periodic2_Profiller++;
}
