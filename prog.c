#include <delay.h>
#include <gpio.h>
#include <stm32.h>

#include "buttons.h"
#include "usart.h"
#include "queue.h"

#include "leds.h"

void turn_on_timing() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA1EN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

int main() {
	turn_on_timing();
	init_buttons();
	init_usart();
	init_leds();
	while(1) {}
	return 0;
}
