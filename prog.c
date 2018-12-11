#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

#define RED_LED_PIN 6
#define GREEN_LED_PIN 7
#define BLUE_LED_PIN 0
#define GREEN2_LED_PIN 5

#define RED_LED_GPIO GPIOA
#define GREEN_LED_GPIO GPIOA
#define BLUE_LED_GPIO GPIOB
#define GREEN2_LED_GPIO GPIOA

#define JOYSTICK_GPIO GPIOB
#define USER_BUTTON_GPIO GPIOC
#define MODE_BUTTON_GPIO GPIOA

#define USER_BUTTON_PIN 13
#define MODE_BUTTON_PIN 0
#define LEFT_BUTTON_PIN 3
#define RIGHT_BUTTON_PIN 4
#define UP_BUTTON_PIN 5
#define DOWN_BUTTON_PIN 6
#define FIRE_BUTTON_PIN 10

#define	FIRE_PRESSED "FIRE PRESSED\n\r"
#define	FIRE_RELEASED "FIRE RELEASED\n\r"

#define	UP_PRESSED "UP PRESSED\n\r"
#define	UP_RELEASED "UP RELEASED\n\r"

#define	RIGHT_PRESSED "RIGHT PRESSED\n\r"
#define	RIGHT_RELEASED "RIGHT RELEASED\n\r"

#define	DOWN_PRESSED "DOWN PRESSED\n\r"
#define	DOWN_RELEASED "DOWN RELEASED\n\r"

#define	LEFT_PRESSED "LEFT PRESSED\n\r"
#define	LEFT_RELEASED "LEFT RELEASED\n\r"

#define	MODE_PRESSED "MODE PRESSED\n\r"
#define	MODE_RELEASED "MODE RELEASED\n\r"

#define	USER_PRESSED "USER PRESSED\n\r"
#define	USER_RELEASED "USER RELEASED\n\r"

#define LOW_STATE 1
#define HIGH_STATE 0

#define TXD_GPIO GPIOA
#define TXD_PIN 2

#define RXD_GPIO GPIOA
#define RXD_PIN 3

#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE
#define USART_WordLength_8b 0x0000
#define USART_Parity_No 0x0000
#define USART_StopBits_1 0x0000
#define USART_FlowControl_None 0x0000
#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ

typedef struct {
    GPIO_TypeDef* gpio;
    uint8_t pin;
} pin_t;

typedef struct {
	pin_t pin;
	uint8_t state;
	uint8_t pressed;
} button_t;

button_t fire = {.pin = {.gpio = JOYSTICK_GPIO, .pin = FIRE_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t up = {.pin = {.gpio = JOYSTICK_GPIO, .pin = UP_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t right = {.pin = {.gpio = JOYSTICK_GPIO, .pin = RIGHT_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t down = {.pin = {.gpio = JOYSTICK_GPIO, .pin = DOWN_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t left = {.pin = {.gpio = JOYSTICK_GPIO, .pin = LEFT_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t mode = {.pin = {.gpio = MODE_BUTTON_GPIO, .pin = MODE_BUTTON_PIN }, .state = LOW_STATE, .pressed = 0 };
button_t user = {.pin = {.gpio = USER_BUTTON_GPIO, .pin = USER_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };

size_t output_buffer_length = 0;
size_t write_length = 0;
char output_buffer[MAX_BUFFER_SIZE];

void button_init(button_t button) {
	GPIOinConfigure(button.pin.gpio, button.pin.pin, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
}

uint8_t pin_test_set_input(pin_t pin) {
	return (pin.gpio->IDR & (1 << pin.pin)) != 0;
}

uint8_t is_button_pressed(button_t button) {
	if (button.state == LOW_STATE) {
		return pin_test_set_input(button.pin);
	} else {
		return !pin_test_set_input(button.pin);
	}
}

uint8_t is_dma_free() {
	return (DMA1_Stream6->CR & DMA_SxCR_EN) == 0 && (DMA1->HISR & DMA_HISR_TCIF6) == 0;
}

void dma_init() {
	DMA1_Stream6->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;
	DMA1_Stream6->PAR = (uint32_t)&USART2->DR;
	DMA1->HIFCR = DMA_HIFCR_CTCIF6;

	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void dma_send(size_t len) {
	DMA1_Stream6->M0AR = (uint32_t)(output_buffer + write_length);
	DMA1_Stream6->NDTR = len;
	DMA1_Stream6->CR |= DMA_SxCR_EN;
}

void usart2_init() {
	GPIOafConfigure(TXD_GPIO,
					TXD_PIN,
					GPIO_OType_PP,
					GPIO_Fast_Speed,
					GPIO_PuPd_NOPULL,
					GPIO_AF_USART2);

	USART2->CR1 = USART_CR1_TE;
	USART2->CR2 = 0;
	USART2->CR3 = USART_CR3_DMAT;
	uint32_t const baudrate = 9600U;
	USART2->BRR = (PCLK1_HZ + (baudrate / 2U)) /
					baudrate;
}

void usart2_enable() {
	USART2->CR1 |= USART_CR1_UE;
}

void usart2_transmit() {
	if (write_length == MAX_BUFFER_SIZE) {
		write_length = 0;
	}

	if (write_length < output_buffer_length || (write_length > output_buffer_length && write_length < MAX_BUFFER_SIZE)) {
		size_t len = (output_buffer_length > write_length) ? output_buffer_length - write_length : MAX_BUFFER_SIZE - write_length;
		dma_send(len);
		write_length += len;
	}
}

void write_to_buffer(char *message) {
	int length = strlen(message);
	size_t first_part = (length + output_buffer_length < MAX_BUFFER_SIZE) ? length : MAX_BUFFER_SIZE - output_buffer_length;
	size_t second_part = (length + output_buffer_length < MAX_BUFFER_SIZE) ? 0 : length + output_buffer_length - MAX_BUFFER_SIZE;

	strncpy(output_buffer + output_buffer_length, message, first_part);
	strncpy(output_buffer, message + first_part, second_part);

	output_buffer_length += (first_part + second_part);
	output_buffer_length %= MAX_BUFFER_SIZE;
}

void process_one_button(button_t *button, char *pressed_message, char *released_message) {
	uint8_t is_pressed = is_button_pressed(*button);

	if (is_pressed != button->pressed) {
		button->pressed = is_pressed;
		
		if (button->pressed) {
			write_to_buffer(pressed_message);
		} else {
			write_to_buffer(released_message);
		}

		if (is_dma_free()) {
			usart2_transmit();
		}
	}
}

void turn_on_timing() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA1EN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

void EXTI15_10_IRQHandler(void) {
	if (EXTI->PR & EXTI_PR_PR13) {
		EXTI->PR = EXTI_PR_PR13;
		process_one_button(&user, USER_PRESSED, USER_RELEASED);
	}

	if (EXTI->PR & EXTI_PR_PR10) {
		EXTI->PR = EXTI_PR_PR10;
		process_one_button(&fire, FIRE_PRESSED, FIRE_RELEASED);
	}
}

void EXTI9_5_IRQHandler(void) {
	if (EXTI->PR & EXTI_PR_PR6) {
		EXTI->PR = EXTI_PR_PR6;
		process_one_button(&down, DOWN_PRESSED, DOWN_RELEASED);
	}

	if (EXTI->PR & EXTI_PR_PR5) {
		EXTI->PR = EXTI_PR_PR5;
		process_one_button(&up, UP_PRESSED, UP_RELEASED);
	}
}

void EXTI4_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR4;
	process_one_button(&right, RIGHT_PRESSED, RIGHT_RELEASED);
}

void EXTI3_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR3;
	process_one_button(&left, LEFT_PRESSED, LEFT_RELEASED);
}

void EXTI0_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR0;
	process_one_button(&mode, MODE_PRESSED, MODE_RELEASED);
}

void DMA1_Stream6_IRQHandler() {
	uint32_t isr = DMA1->HISR;
	
	if (isr & DMA_HISR_TCIF6) {
		DMA1->HIFCR = DMA_HIFCR_CTCIF6;
		usart2_transmit();
	}
}

void buttons_init() {
	button_init(fire);
	button_init(up);
	button_init(right);
	button_init(down);
	button_init(left);
	button_init(mode);
	button_init(user);

	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
}

int main() {
	turn_on_timing();
	buttons_init();
	usart2_init();
	dma_init();

	usart2_enable();

	while(1) {}
	return 0;
}
