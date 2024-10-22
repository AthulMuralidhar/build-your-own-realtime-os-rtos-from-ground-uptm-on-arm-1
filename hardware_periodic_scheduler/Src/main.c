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
#define TIM2_SR_UIF		(1<<0)

uint32_t Periodic2_Profiller;

int main(void) {
	// initialize hardware timer
	TIM2_1hz_interrupt_init();

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
