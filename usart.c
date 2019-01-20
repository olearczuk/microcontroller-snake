#include <gpio.h>
#include <string.h>

#include "usart.h"
#include "queue.h"
#include "leds.h"

#define USART_FlowControl_None 0x0000
#define USART_StopBits_1 0x0000
#define USART_Parity_No 0x0000
#define USART_WordLength_8b 0x0000
#define USART_Enable USART_CR1_UE
#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)

#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ

#define USART_GPIO GPIOA
#define USART1_TXD_PIN 9
#define USART1_RXD_PIN 10

#define OUTPUT_BUFFER_SIZE 100
#define INPUT_BUFFER_SIZE 1

char output_buffer[OUTPUT_BUFFER_SIZE];
char input_buffer[INPUT_BUFFER_SIZE];
Queue queue;

static void turn_clock_on_usart() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

static void configure_gpio_usart() {
	GPIOafConfigure(USART_GPIO, USART1_TXD_PIN, GPIO_OType_PP, GPIO_Fast_Speed,
					GPIO_PuPd_NOPULL, GPIO_AF_USART1);
	GPIOafConfigure(USART_GPIO, USART1_RXD_PIN, GPIO_OType_PP, GPIO_Fast_Speed, 
					GPIO_PuPd_UP, GPIO_AF_USART1);
}

static void set_registers_usart() {
	uint32_t const baudrate = 9600U;
	USART1->CR1 = USART_Mode_Rx_Tx | USART_WordLength_8b | USART_Parity_No;
	USART1->CR2 = USART_StopBits_1;
	USART1->BRR = (PCLK1_HZ + (baudrate / 2U)) / baudrate;
	USART1->CR3 = USART_CR3_DMAT | USART_CR3_DMAR;
}

static void configure_usart() {
	turn_clock_on_usart();
	configure_gpio_usart();
	set_registers_usart();
}

static void turn_clock_on_dma() {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}

static void set_dma_streams() {
	DMA2_Stream7->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | 
					   DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;
    DMA2_Stream7->PAR = (uint32_t)&USART1->DR;

    DMA2_Stream5->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_TCIE;
    DMA2_Stream5->PAR = (uint32_t)&USART1->DR;
}

static void init_dma_interrupt() {
	DMA2->HIFCR = DMA_HIFCR_CTCIF7 | DMA_HIFCR_CTCIF5;
	NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);

}

static void init_dma() {
	turn_clock_on_dma();
	set_dma_streams();
	init_dma_interrupt();
}

static void enable_usart() {
	USART1->CR1 |= USART_Enable;
}

static int is_dma_busy() {
	return DMA2_Stream7->CR & DMA_SxCR_EN;
}

static void send_dma(char *message, uint32_t len) {
	DMA2_Stream7->M0AR = (uint32_t) message;
	DMA2_Stream7->NDTR = len;
	DMA2_Stream7->CR |= DMA_SxCR_EN;
}

static void recv_dma() {
	DMA2_Stream5->M0AR = (uint32_t)input_buffer;
	DMA2_Stream5->NDTR = INPUT_BUFFER_SIZE;
	DMA2_Stream5->CR |= DMA_SxCR_EN;
}

void init_usart() {
	configure_usart();
	init_dma();
	enable_usart();
	init_queue(&queue);
	recv_dma();
}

void send_usart(char *message) {
	if (is_dma_busy())
		queue_put(&queue, message);
	else
		send_dma(message, strlen(message));
}

void DMA2_Stream7_IRQHandler() {
	uint32_t isr = DMA2->HISR;
	
	if (isr & DMA_HISR_TCIF7) {
		DMA2->HIFCR = DMA_HIFCR_CTCIF7;
		
		if (!queue_is_empty(&queue)) {
			uint32_t len = queue_get(&queue, output_buffer, OUTPUT_BUFFER_SIZE);
			send_dma(output_buffer, len);
		}
	}
}


void DMA2_Stream5_IRQHandler() {
	uint32_t isr = DMA2->HISR;

	if (isr & DMA_HISR_TCIF5) {
		DMA2->HIFCR = DMA_HIFCR_CTCIF5;

		parse_led(input_buffer);
		recv_dma();
	}
}