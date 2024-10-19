/* Developer notes:
 *
 * User LD3: orange LED is a user LED connected to the I/O PD13 of the
 * STM32F407VGT6.
 *
 * GPIO port D 13 is connected to AHB1 bus
 * RCC_AHB1ENR offset is 0x30
 *
 * Bit 3 GPIODEN: IO port D clock enable of the RCC_AHB1ENR register
 *
 * - LEDs are connected from  PD 12 to PD 15

 * */

#include "stm32f407xx.h"

#define GPIOD_EN		(1 << 3)
#define LED_ON			(1<<13)

void led_init();
void led_on();
void led_off();

int main(void) {
	led_init();

	while(1) {
		led_on();
		for(int i = 0; i < 900000; i++);
		led_off();
		for(int i = 0; i < 900000; i++);
	}

}

void led_init() {
	// 1. enable AHB1 clock
	RCC->AHB1ENR |= GPIOD_EN;

	// 2. set LED pin as output mode
	GPIOD->MODER |= (1 << 12);
	GPIOD->MODER &= ~(1 << 13);

}

void led_on() {
	// 1. set led pin to high
	GPIOD->ODR |= LED_ON;
}

void led_off() {
// 1. set led pin to low
	GPIOD->ODR &= ~LED_ON;
}
