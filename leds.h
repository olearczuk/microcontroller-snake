#define RED_LED_GPIO    GPIOA
#define RED_LED_PIN    6

#define RedLEDon() \
	RED_LED_GPIO->BSRRH = 1 << RED_LED_PIN
#define RedLEDoff() \
	RED_LED_GPIO->BSRRL = 1 << RED_LED_PIN
#define RedLEDToggle() \
        RED_LED_GPIO->ODR ^= 1 << RED_LED_PIN


void init_leds() {
		__NOP();
	RedLEDoff();
	
	GPIOoutConfigure(RED_LED_GPIO,
					 RED_LED_PIN,
					 GPIO_OType_PP,
					 GPIO_Low_Speed,
					 GPIO_PuPd_NOPULL);
}