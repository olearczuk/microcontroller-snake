#include <string.h>
#include <gpio.h>

#include "usart.h"
#include "buttons.h"
#include "leds.h"

#define JOYSTICK_GPIO GPIOB

#define LEFT_BUTTON_PIN 3
#define RIGHT_BUTTON_PIN 4
#define UP_BUTTON_PIN 5
#define DOWN_BUTTON_PIN 6

#define	UP_PRESSED "U"

#define	RIGHT_PRESSED "R"

#define	DOWN_PRESSED "D"

#define	LEFT_PRESSED "L"

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

static void init_button(button_t button) {
	GPIOinConfigure(button.pin.gpio, button.pin.pin, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
}

void init_buttons() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	init_button(up);
	init_button(right);
	init_button(down);
	init_button(left);

	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
}

static uint8_t pin_test_set_input(pin_t pin) {
	return (pin.gpio->IDR & (1 << pin.pin)) != 0;
}

static uint8_t is_button_pressed(button_t button) {
	if (button.state == LOW_STATE) {
		return pin_test_set_input(button.pin);
	} else {
		return !pin_test_set_input(button.pin);
	}
}

static void process_one_button(button_t *button, char *message) {
	uint8_t is_pressed = is_button_pressed(*button);
	if (is_pressed != button->pressed) {
		button->pressed = is_pressed;

		if (is_pressed) {
			send_usart(message);
		}
	}
}

void EXTI9_5_IRQHandler(void) {
	if (EXTI->PR & EXTI_PR_PR6) {
		EXTI->PR = EXTI_PR_PR6;
		process_one_button(&down, DOWN_PRESSED);
	}

	if (EXTI->PR & EXTI_PR_PR5) {
		EXTI->PR = EXTI_PR_PR5;
		process_one_button(&up, UP_PRESSED);
	}
}

void EXTI4_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR4;
	process_one_button(&right, RIGHT_PRESSED);
}

void EXTI3_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR3;
	process_one_button(&left, LEFT_PRESSED);
}