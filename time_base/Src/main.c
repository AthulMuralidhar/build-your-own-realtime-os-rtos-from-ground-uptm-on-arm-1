/* DEVELOPER NOTES:
 *
 * SYST_CSR controls the functioning of the SysTick clock
 *
 *
 *
 *
 * */

#include <stdint.h>
#include "./chip-headers/stm32f407xx.h"
#include "printer.h"

#define  SYSTICK_ENABLE		(1<<0)
#define  SYSTICK_TICKINT		(1<<1)
#define  SYSTICK_CLKSOURCE		(1<<2)  // set to processor clock
#define  SYSTICK_COUNTFLAG		(1<<16)
#define ONCE_SECOND_LOAD		16000000
#define  MAX_DELAY		0xFFFFFFFF

volatile uint32_t g_current_tick;
volatile uint32_t g_current_tick_prime;
volatile uint32_t tick_frequency = 1;

void timebase_init();
void tick_increment();
void delay(uint32_t delay);
uint32_t get_tick();

int main(void) {
	timebase_init();

	while(1) {
		delay(1);
		ITMPrint("a second occurred\n\r");
	}
}

void SysTick_Handler() {
	tick_increment();
}


void delay(uint32_t delay) {
	uint32_t tick_start = get_tick();

	if (delay < MAX_DELAY) {
		delay += tick_frequency;
	}

	while (get_tick() - tick_start < delay);

}

uint32_t get_tick() {
	__disable_irq();
	g_current_tick_prime = g_current_tick;
	__enable_irq();

	return g_current_tick_prime;
}

void tick_increment() {
	g_current_tick += tick_frequency;
}

// create a timeout each second
void timebase_init() {
	// 1. reload the timer with number of cycles per second
	SysTick->LOAD = ONCE_SECOND_LOAD - 1;  // count from 0

	// 2. clear SysTick current value register
	SysTick->VAL = 0;

	// 3. select internal clock source
	SysTick->CTRL = SYSTICK_CLKSOURCE;

	// 4. enable interrupt
	SysTick->CTRL |= SYSTICK_TICKINT;

	// 5. enable SysTick
	SysTick->CTRL |= SYSTICK_ENABLE;

	// 6. enable global interrupts
	__enable_irq();

}
