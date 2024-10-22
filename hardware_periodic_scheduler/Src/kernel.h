/*
 * kernel.h
 *
 *  Created on: Oct 22, 2024
 *      Author: athul-muralidhar
 */

#include "./chip-headers/stm32f407xx.h"
#include "tasks.h"

#ifndef KERNEL_H_
#define KERNEL_H_

#define NUM_OF_THREADS		3
#define STACK_SIZE			100   // 100 32 bit values
#define BUS_FREQ			16000000  // 16Mhz
#define MILLI_SEC_PRE_SCALAR		((BUS_FREQ) / (1000))
#define  SYSTICK_ENABLE		(1<<0)
#define  SYSTICK_TICKINT		(1<<1)  // enable SysTick interrupt
#define  SYSTICK_CLKSOURCE		(1<<2)  // set to processor clock
#define  SYSTICK_COUNTFLAG		(1<<16)
#define SYSTICK_RESET			0
#define ICSR_BASE		(*(volatile uint32_t*)0xE000ED04)
#define ICSR_PENDSTSET		(1 << 26)
#define TASK_PERIOD		100
#define TIM2_FREQ		1  // 1hz
#define TIM2_EN		(1<<0)
#define CR1_CEN 		(1<<0)
#define DIER_UPDATE_INTERRUPT_ENABLE 		(1 << 0)


typedef struct TCB {
	int32_t *stackPtr;
	struct TCB *next;

} TCB;

TCB tcbs[NUM_OF_THREADS];
TCB *current;

// each thread has stack size of 100 i.e 400 bytes
int32_t TCB_STACK[NUM_OF_THREADS][STACK_SIZE];
uint32_t period_tick;

void task3();

void os_scheduler_launch() {
	// SET THE STACK POINTER
	// 1. load address of current onto R0
	__asm volatile ("LDR R0,=current");
	// 2. load r2, r2 = current
	__asm volatile ("LDR R2,[R0]");
	// 3. load cortex stack pointer from r2, SP=current->stackPtr
	__asm volatile ("LDR SP,[R2]");

	// DISCARD THE LR AND PSR FROM THE INITIAL STACK, LOAD THE VALUES WE WANT AFTER
	// 4. restore r4-r11 manually
	__asm volatile ("POP {R4-R11}");

	// restore r12 - LR
	__asm volatile ("POP {R12}");

	// 5. restore r0-r3
	__asm volatile ("POP {R0-R3}");
	// 6. skip LR
	__asm volatile ("ADD SP,SP,#4");
	// 7. create a new start location by popping LR
	__asm volatile ("POP {LR}");
	// 8. skip PSR
	__asm volatile ("ADD SP,SP,#4");
	// 9. enable global interrupts
	__asm volatile ("CPSIE  I");
	// 10. return from  exception
	__asm volatile ("BX  LR");

}

__attribute__((naked)) void SysTick_Handler() {
	// A. suspend current thread
	// 2. disable global interrupts
	__asm volatile ("CPSID  I");

	// 3. save r4-r11 manually
	__asm volatile ("PUSH {R4-R11}");

	// 4. load the address of the current TCB into r0
	__asm volatile ("LDR R0,=current");

	// 5. load R1 from address= R0
	__asm volatile ("LDR R1,[R0]");

	// 6. store Cortex-M  stack pointer at R1
	__asm volatile ("STR SP,[R1]");

	// B. choose next thread
	// 1. load R1 from a location 4 bytes above the address at r1 - r1 = current->next
	// save r0 LR as we are branching from here to the os_schedular_round_robin function
	__asm volatile ("PUSH {R0,LR}");
	__asm volatile ("BL os_schedular_round_robin");  // branch
	// restore R0 and LR back
	__asm volatile ("POP {R0,LR}");
//
//	// 2. store R1 at r0-current = r1
//	__asm volatile ("STR R1,[R0]");
//
//	// 3. load cortex m stack pointer from r1 - SP = current->stackPtr
//	__asm volatile("LDR SP,[R1]");

	// r1 = current -new thread
	__asm volatile ("LDR R1,[R0]");

	// stack point = current->stackptr
	__asm volatile ("LDR SP,[R1]");

	// 4. restore r4-r11
	__asm volatile ("POP {R4-R11}");

	// 5. enable global interrupts
	__asm volatile ("CPSIE  I");

	// c. return from  exception and restore r0-r3, LR, PC, PSR
	__asm volatile ("BX  LR");

}

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

	tcbs[thread_number].stackPtr = &TCB_STACK[thread_number][STACK_SIZE - 16]; // stack pointer i.e top of stack

	// 1. set bit 24 - the T bit in xPSR to 1 (thumb mode)
	TCB_STACK[thread_number][STACK_SIZE - 1] = (1 << 24);  // xPSR register

	// dummy inits - this for loop is optional - for DEBUG only
	for (int i = 3; i < 17; i++) {
		// TCB_STACK[thread_number][STACK_SIZE -3] = 0xAAAAAAAA is Link Register LR
		// TCB_STACK[thread_number][STACK_SIZE -i] = 0xAAAAAAAA; is R12 to R4
		// more info on Cortex generic user guide pg: 16
		TCB_STACK[thread_number][STACK_SIZE - i] = 0xAAAAAAAA;
	}
}

uint8_t os_kernel_add_threads(void (*task0), void (*task1), void (*task2)) { // he does it like this: void(*task2) (void)  - will change to that if the current style causes problems
	// 1. disable global interrupts
	__disable_irq();
	tcbs[0].next = &tcbs[1];
	tcbs[1].next = &tcbs[2];
	tcbs[2].next = &tcbs[0];

	// 2a. initialize stack for thread 0
	os_kernel_stack_init(0);
	// 3a. initialize Program counter PC for thread 0
	TCB_STACK[0][STACK_SIZE - 2] = (uint32_t) task0;

	// 2b. initialize stack for thread 1
	os_kernel_stack_init(1);
	// 3b. initialize Program counter PC for thread 0
	TCB_STACK[1][STACK_SIZE - 2] = (uint32_t) task1;

	// 2c. initialize stack for thread 2
	os_kernel_stack_init(2);
	// 3c. initialize Program counter PC for thread 0
	TCB_STACK[2][STACK_SIZE - 2] = (uint32_t) task2;

	// 4. Initialize current to the task 0
	current = &tcbs[0];

	// 5. enable global interrupts back
	__enable_irq();

	return 1;   // dunno y tho
}

void os_kernel_init() {

}

void os_thread_yield() {
	// 1. clear systick current value register
	SysTick->VAL = 0;

	// 2. trigger systick
	ICSR_BASE = ICSR_PENDSTSET;
}

void os_schedular_round_robin() {
	if (period_tick == TASK_PERIOD) {
		(*task3)();
		period_tick = 0;
	}
	period_tick++;

	current = current->next;
}

void TIM2_1hz_interrupt_init() {
	// enable clock access
	RCC->APB1ENR |= TIM2_EN;

	// set the timer pre-scalar
	TIM2->PSC = 1600 -1;  // 16 000 000 / 1600  == 10 000

	// set auto reload value
	TIM2->ARR = 10000 -1;   // 10 000 / 10 000  == 1hz

	// clear the timer counter
	TIM2->CNT = 0;


	// enable the timer
	TIM2->CR1  |= CR1_CEN;

	// enable timer interrupt
	TIM2->DIER |= DIER_UPDATE_INTERRUPT_ENABLE;

	// enable timer interrupt in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
}



#endif /* KERNEL_H_ */
