// #include <delay.h>
#include <gpio.h>

#include "buttons.h"

void turn_on_timing() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA1EN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

int main() {
	turn_on_timing();
	buttons_init();
	usart2_init();
	dma_init();

	init_leds();

	usart2_enable();

	DMA1_Stream5->M0AR = (uint32_t)input_buffer;
	DMA1_Stream5->NDTR = 1;
	DMA1_Stream5->CR |= DMA_SxCR_EN;

	while(1) {}
	return 0;
}
