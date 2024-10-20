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
 *
 *
 * In RCC_APB1ENR -  bit position 17 sets USART2 to enabled
 *
 * in the USART_CR1 you  have transmitter enable - TE at bit 3
 * in the USART_CR1 you  have receiver enable - RE at bit 2
 *
 *
 * USART_SR is the status register for USART - we need to check bit 7 TXE
 * which tells us if the transmit data register is empty. after this we can write
 * to the transmit data register
 *
 * */

#include "stm32f407xx.h"
#include <stdio.h>
#include <stdint.h>

#define GPIOA_EN		(1 << 0)
#define UART2_EN		(1 << 17)
#define SYSTEM_CLK_FREQUENCY		16000000
#define APB1_CLK 		SYSTEM_CLK_FREQUENCY
#define UART_BAUD_RATE		115200
//#define USART_CR1_TE		(1 <<  3)   // already defined in stm32f407xx.h"
//#define USART_CR1_UE		(1 <<  13)  // already defined in stm32f407xx.h"
//#define USART_SR_TXE		(1 <<  7)   // already defined in stm32f407xx.h"



void uart_tx_init();
void uart_write(int ch);
uint32_t compute_baud_rate(uint32_t periph_clock, uint32_t baud_rate);

int main(void) {
	uart_tx_init();

	while(1) {
		printf("testing 123 123 123\n\r");
	}
}

int __io_putchar(int ch) {
	uart_write(ch);
	return ch;
}

void uart_write(int ch) {
	// 1. make sure that the transmit data register is empty
	while(!(USART2->SR & USART_SR_TXE));  // wait for the status register to show empty transmit register

	// 2. write data to transmit data register
	// FIXME: the data register is not registering this value
	// the value below is 101 but it shows 0 for some reason
	// apparently this is not a bug - this is in write mode and we cant
	// read the data here - as it would only show 0 - the RE (read enabled) is not set
	USART2->DR = (ch & 0xFF);

	// Wait for TC flag to be set, indicating transmission is complete
	while (!(USART2->SR & USART_SR_TC));
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
	RCC->APB1ENR |= UART2_EN;

	// 5. set the baud rate
	USART2->BRR = compute_baud_rate(APB1_CLK, UART_BAUD_RATE);

	// 6. configure transfer direction
	USART2->CR1 = USART_CR1_TE;  // we want to set everything to 0 except the TE bit at bit position 3

	// 7. enable UART module
	USART2->CR1 |= USART_CR1_UE;
}


uint32_t compute_baud_rate(uint32_t periph_clock, uint32_t baud_rate) {
	return (periph_clock + (baud_rate/2))/baud_rate;
}

