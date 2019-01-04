#include <string.h>

#include "dma.h"

#define JOYSTICK_GPIO GPIOB

#define LEFT_BUTTON_PIN 3
#define RIGHT_BUTTON_PIN 4
#define UP_BUTTON_PIN 5
#define DOWN_BUTTON_PIN 6

#define	UP_PRESSED "UP PRESSED\n\r"
#define	UP_RELEASED "UP RELEASED\n\r"

#define	RIGHT_PRESSED "RIGHT PRESSED\n\r"
#define	RIGHT_RELEASED "RIGHT RELEASED\n\r"

#define	DOWN_PRESSED "DOWN PRESSED\n\r"
#define	DOWN_RELEASED "DOWN RELEASED\n\r"

#define	LEFT_PRESSED "LEFT PRESSED\n\r"
#define	LEFT_RELEASED "LEFT RELEASED\n\r"

#define LOW_STATE 1
#define HIGH_STATE 0

typedef struct {
    GPIO_TypeDef* gpio;
    uint8_t pin;
} pin_t;

typedef struct {
	pin_t pin;
	uint8_t state;
	uint8_t pressed;
} button_t;

button_t up = {.pin = {.gpio = JOYSTICK_GPIO, .pin = UP_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t right = {.pin = {.gpio = JOYSTICK_GPIO, .pin = RIGHT_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t down = {.pin = {.gpio = JOYSTICK_GPIO, .pin = DOWN_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };
button_t left = {.pin = {.gpio = JOYSTICK_GPIO, .pin = LEFT_BUTTON_PIN }, .state = HIGH_STATE, .pressed = 0 };

void button_init(button_t button) {
	GPIOinConfigure(button.pin.gpio, button.pin.pin, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
}

void buttons_init() {
	button_init(up);
	button_init(right);
	button_init(down);
	button_init(left);

	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
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