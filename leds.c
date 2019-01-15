#include <gpio.h>

#include "leds.h"

#define RED_LED_GPIO GPIOA
#define RED_LED_PIN 6
#define RED_LED_ON '1'
#define RED_LED_OFF '2'

#define RedLEDon() \
	RED_LED_GPIO->BSRRH = 1 << RED_LED_PIN
#define RedLEDoff() \
	RED_LED_GPIO->BSRRL = 1 << RED_LED_PIN
#define RedLEDToggle() \
    RED_LED_GPIO->ODR ^= 1 << RED_LED_PIN


void init_leds() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	GPIOoutConfigure(RED_LED_GPIO,
					 RED_LED_PIN,
					 GPIO_OType_PP,
					 GPIO_Low_Speed,
					 GPIO_PuPd_NOPULL);
	RedLEDoff();
}

void parse_led(char *message) {
	if (message[0] == RED_LED_ON)
		RedLEDon();
	else if (message[0] == RED_LED_OFF)
		RedLEDoff();
}