/* developer notes
 *
 * USART2 is hanging on alternate function mappings as here:
 * USART2_CTS	-	PA0
 * USART2_RTS	-	PA1
 * USART2_TX	-	PA2  // this is what we want
 * USART2_RX	-	PA3
 * USART2_CK	-	PA4
 * all the USART / UART are AF7
 *
 *
 * PA ports belong to GPIO Port A
 *
 * PA2 in GPIOA_MODER has a MODER2 at 5,4 bit positions
 * so bit position 5 == 1 and bit position 4 == 0
 *
 * in the GPIOA_AFRL, the port PA2  AFRL2[3:0] - we have to
 * set 0111 in this  bit positions 8 TO 11 for AF7
 * */

#include "stm32f407xx.h"

#define GPIOA_EN		(1 << 0)

void uart_tx_init();

int main(void) {
	/* Loop forever */
	for (;;)
		;
}

void uart_tx_init() {
	// 1. enable clock access to GPIO A
	RCC->AHB1ENR |= GPIOA_EN;

	// 2. Set PA2 to alternate function  mode
	GPIOA->MODER |= (1 << 5);
	GPIOA->MODER &= ~(1 << 4);

	// 3. set alternate function type to AF7
	GPIOA->AFR[0] &= ~(1 << 11);
	GPIOA->AFR[0] |= (1 << 10);
	GPIOA->AFR[0] |= (1 << 9);
	GPIOA->AFR[0] |= (1 << 8);

	// 4. enable clock access to UART2
	// 5. set the baud rate
	// 6. configure transfer direction
	// 7. enable UART module
}
