/*
 * DEVELOPER NOTES
 *
 *
 *
 * */


#include <stdint.h>
#include "./chip-headers/stm32f407xx.h"

// ==================== KERNAL CODE ===================

#define NUM_OF_THREADS		3
#define STACK_SIZE			100   // 100 32 bit values
#define BUS_FREQ			16000000  // 16Mhz
#define MILLI_SEC_PRE_SCALAR		((BUS_FREQ) / (1000))
#define  SYSTICK_ENABLE		(1<<0)
#define  SYSTICK_TICKINT		(1<<1)  // enable SysTick interrupt
#define  SYSTICK_CLKSOURCE		(1<<2)  // set to processor clock
#define  SYSTICK_COUNTFLAG		(1<<16)
#define SYSTICK_RESET			0

typedef struct {
	int32_t *stackPtr;
	struct TCB *next;

} TCB;

TCB tcbs[NUM_OF_THREADS];
TCB *current;

// each thread has stack size of 100 i.e 400 bytes
int32_t	TCB_STACK[NUM_OF_THREADS][STACK_SIZE];


void os_kernel_launch(uint32_t quantum) {
	// 1. reset SysTick
	SysTick->CTRL = SYSTICK_RESET;

	// 2. clear SysTick current value register
	SysTick->VAL = SYSTICK_RESET;

	// 3. load time slice / quantum
	SysTick->LOAD = (quantum * MILLI_SEC_PRE_SCALAR - 1);

	// 4. set SysTick to low priority
	NVIC_SetPriority(SysTick_IRQn, 15);

	// 5. enable SysTick, select internal clock
	SysTick->CTRL = SYSTICK_CLKSOURCE;
	SysTick->CTRL |= SYSTICK_ENABLE;

	// 6. enable SysTick interrupt
	SysTick->CTRL |= SYSTICK_TICKINT;

	// 7. launch scheduler
	os_scheduler_launch();

}

void os_kernel_stack_init(int thread_number) {

	tcbs[thread_number].stackPtr = &TCB_STACK[thread_number][STACK_SIZE - 16];    // stack pointer i.e top of stack

	// 1. set bit 21 - the T bit in xPSR to 1 (thumb mode)
	// NOTE: the generic user guide says its the 24th bit
	TCB_STACK[thread_number][STACK_SIZE - 1]= (1 << 21);  // xPSR register

	// dummy inits - this for loop is optional - for DEBUG only
	for(int i=3; i < 17; i++) {
		// TCB_STACK[thread_number][STACK_SIZE -3] = 0xAAAAAAAA is Link Register LR
		// TCB_STACK[thread_number][STACK_SIZE -i] = 0xAAAAAAAA; is R12 to R4
		// more info on Cortex generic user guide pg: 16
		TCB_STACK[thread_number][STACK_SIZE -i] = 0xAAAAAAAA;
	}
}

uint8_t os_kernel_add_threads(void(*task0),void(*task1),void(*task2) ) {  // he does it like this: void(*task2) (void)  - will change to that if the current style causes problems
	// 1. disable global interrupts
	__disable_irq();
	tcbs[0].next = &tcbs[1];
	tcbs[1].next = &tcbs[2];
	tcbs[2].next = &tcbs[0];

	// 2a. initialize stack for thread 0
	os_kernel_stack_init(0);
	// 3a. initialize Program counter PC for thread 0
	TCB_STACK[0][STACK_SIZE - 2] = (uint32_t)task0;

	// 2b. initialize stack for thread 1
	os_kernel_stack_init(1);
	// 3b. initialize Program counter PC for thread 0
	TCB_STACK[1][STACK_SIZE - 2] = (uint32_t)task1;

	// 2c. initialize stack for thread 2
	os_kernel_stack_init(2);
	// 3c. initialize Program counter PC for thread 0
	TCB_STACK[2][STACK_SIZE - 2] = (uint32_t)task2;


	// 4. Initialize current to the task 0
	current = &tcbs[0];

	// 5. enable global interrupts back
	__enable_irq();

	return 1;   // dunno y tho
}

void os_kernel_init() {

}


// ======================= 	MAIN ==============================

int main(void)
{
    /* Loop forever */
	for(;;);
}


