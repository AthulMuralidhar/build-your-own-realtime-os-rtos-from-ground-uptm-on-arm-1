/*
 * DEVELOPER NOTES
 *
 *
 *
 * */

#include <stdint.h>
#include "./chip-headers/stm32f407xx.h"
#include "printer.h"

// ==================== KERNAL CODE ===================

#define NUM_OF_THREADS		3
#define STACK_SIZE			100   // 100 32 bit v  alues
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
int32_t TCB_STACK[NUM_OF_THREADS][STACK_SIZE];

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
	__asm volatile("LDR R1,[R1,#4]");

	// 2. store R1 at r0-current = r1
	__asm volatile ("STR R1,[R0]");

	// 3. load cortex m stack pointer from r1 - SP = current->stackPtr
	__asm volatile("LDR SP,[R1]");

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

// ======================= TASKS  ===========================

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
	}
}

void task1() {
	while (1) {
		Task1_profiler++;
	}
}

void task2() {
	while (1) {
		Task2_profiler++;
	}
}

// ======================= 	MAIN ==============================
#define QUANTUM		10

int main(void) {
	os_kernel_init();
	os_kernel_add_threads(&task0, &task1, &task2);

	// set time quantum
	os_kernel_launch(QUANTUM);

}

