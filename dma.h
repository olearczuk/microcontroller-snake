#include "leds.h"

#define MAX_BUFFER_SIZE 1024

#define TXD_GPIO GPIOA
#define TXD_PIN 2

#define RXD_GPIO GPIOA
#define RXD_PIN 3

#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ

size_t output_buffer_length = 0;
size_t write_length = 0;
char output_buffer[MAX_BUFFER_SIZE];
char input_buffer[5];

void dma_init() {
	DMA1_Stream6->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;
	DMA1_Stream6->PAR = (uint32_t)&USART2->DR;
	
	DMA1_Stream5->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_TCIE;
	DMA1_Stream5->PAR = (uint32_t)&USART2->DR;
	
	DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CTCIF5;

	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

void usart2_init() {
	GPIOafConfigure(TXD_GPIO,
					TXD_PIN,
					GPIO_OType_PP,
					GPIO_Fast_Speed,
					GPIO_PuPd_NOPULL,
					GPIO_AF_USART2);

	GPIOafConfigure(GPIOA,
					RXD_PIN,
					GPIO_OType_PP,
					GPIO_Fast_Speed,
					GPIO_PuPd_UP,
					GPIO_AF_USART2);

	USART2->CR1 = USART_CR1_TE;
	USART2->CR2 = 0;
	USART2->CR3 = USART_CR3_DMAT | USART_CR3_DMAR;
	uint32_t const baudrate = 9600U;
	USART2->BRR = (PCLK1_HZ + (baudrate / 2U)) /
					baudrate;
}

void usart2_enable() {
	USART2->CR1 |= USART_CR1_UE;
}

uint8_t is_dma_free() {
	return (DMA1_Stream6->CR & DMA_SxCR_EN) == 0 && (DMA1->HISR & DMA_HISR_TCIF6) == 0;
}

void usart2_transmit() {
	if (write_length == MAX_BUFFER_SIZE) {
		write_length = 0;
	}

	if (write_length < output_buffer_length || (write_length > output_buffer_length && write_length < MAX_BUFFER_SIZE)) {
		size_t len = (output_buffer_length > write_length) ? output_buffer_length - write_length : MAX_BUFFER_SIZE - write_length;
		
		DMA1_Stream6->M0AR = (uint32_t)(output_buffer + write_length);
		DMA1_Stream6->NDTR = len;
		DMA1_Stream6->CR |= DMA_SxCR_EN;

		write_length += len;
	}
}

void DMA1_Stream6_IRQHandler() {
	uint32_t isr = DMA1->HISR;
	
	if (isr & DMA_HISR_TCIF6) {
		DMA1->HIFCR = DMA_HIFCR_CTCIF6;
		RedLEDToggle();
		usart2_transmit();
	}
}


// void dma_receive() {
// 	GreenLEDon();
// 	if (input_buffer[0] == '1')
// 		GreenLEDon();
// 	else if (input_buffer[0] == '2')
// 		GreenLEDoff();
// 	else if (input_buffer[0] == '3')
// 		RedLEDon();
// 	else if (input_buffer[0] == '4')
// 		RedLEDoff();
// 	DMA1_Stream5->M0AR = (uint32_t)input_buffer;
// 	DMA1_Stream5->NDTR = 1;
// 	DMA1_Stream5->CR |= DMA_SxCR_EN;
// }

void DMA1_Stream5_IRQHandler() {
	/* Odczytaj zgłoszone przerwania DMA1. */
	uint32_t isr = DMA1->HISR;
	if (isr & DMA_HISR_TCIF5) {
		RedLEDToggle();
		DMA1->HIFCR = DMA_HIFCR_CTCIF5;
	}
}