#include "buttons.h"
#include <gpio.h>

static void turn_clock_on_timer() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
}

static void set_registers_timer() {
	TIM3->CR1 = 0;
	TIM3->PSC = 15000;
	TIM3->ARR = 15;
	TIM3->EGR = TIM_EGR_UG;
}

static void set_interrupts_timer() {
	TIM3->SR = ~TIM_SR_UIF;
	TIM3->DIER = TIM_DIER_UIE;

	NVIC_EnableIRQ(TIM3_IRQn);
}

static void stop_timer() {
	TIM3->CR1 &= ~TIM_CR1_CEN;
	TIM3->CNT = 0;
}

void init_timer() {
	turn_clock_on_timer();
	set_registers_timer();
	set_interrupts_timer();
}

void start_timer() {
	TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM3_IRQHandler(void) {
	uint32_t it_status = TIM3->SR & TIM3->DIER;
	if (it_status & TIM_SR_UIF) {
		TIM3->SR = ~TIM_SR_UIF;
		stop_timer();
		check_buttons();
	}
}