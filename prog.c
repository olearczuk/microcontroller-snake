#include <delay.h>
#include <gpio.h>
#include <stm32.h>

#include "buttons.h"
#include "usart.h"
#include "queue.h"

#include "leds.h"

int main() {
	init_leds();
	init_usart();
	init_buttons();
	while (1) {}
	return 0;
}
