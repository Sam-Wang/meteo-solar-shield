#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/iwdg.h>

#include "sensor.h"
#include "can.h"

#define LED_PORT GPIOB
#define LED_PIN GPIO0

/*
#define SEMIHOSTING
*/


extern void initialise_monitor_handles(void);


static void clock_setup(void) {
	/* Setup SYSCLK to 4MHz (HSE is running at 16MHz). */
	rcc_osc_on(RCC_HSE);
	rcc_wait_for_osc_ready(RCC_HSE);
	rcc_set_sysclk_source(RCC_HSE);
	rcc_set_hpre(RCC_CFGR_HPRE_DIV4);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
}


static void gpio_setup(void) {
	/* Initialize the STAT LED. */
	gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}


static void periodic_timer_setup(void) {
	rcc_periph_clock_enable(RCC_TIM3);

	timer_reset(TIM3);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_continuous_mode(TIM3);
	timer_direction_up(TIM3);
	timer_disable_preload(TIM3);
	timer_enable_update_event(TIM3);
	/* Tick every millisecond. */
	timer_set_prescaler(TIM3, 3999);
	timer_set_period(TIM3, 2000);

	timer_disable_oc_preload(TIM3, TIM_OC1);
	timer_set_oc_value(TIM3, TIM_OC1, 0);
	timer_enable_irq(TIM3, TIM_DIER_CC1IE);

	nvic_enable_irq(NVIC_TIM3_IRQ);
	timer_enable_counter(TIM3);
}


void tim3_isr(void) {
	if (TIM_SR(TIM3) & TIM_SR_CC1IF) {
		timer_clear_flag(TIM3, TIM_SR_CC1IF);

		/* The blue LED is lit while we iterate over the all sensors
		 * and measure. */
		gpio_set(LED_PORT, LED_PIN);

		/* Reset and reconfigure the I2C bus before the measurement. This is
		 * done to (hopefully) avoid all the I2C random troubles. */
		sensor_setup();
		sensor_measure_all();
		gpio_clear(LED_PORT, LED_PIN);
	}
}


static void delay(uint32_t l) {
	for (uint32_t i = 0; i < l; i++) {
		__asm__("nop");
	}
}


int main(void) {

	clock_setup();
	#if defined(SEMIHOSTING)
		initialise_monitor_handles();
	#endif
	gpio_setup();
	sensor_setup();
	can_setup();

	/* Wait a bit before trying to detect sensors. */
	delay(1000000);

	/* But allow only few seconds to detect them. */
	iwdg_set_period_ms(10000);
	iwdg_start();

	sensor_detect_all_buses();
	if (sensor_count == 0) {
		/* If no sensors were detected, do a single long LED blink. */
		gpio_set(LED_PORT, LED_PIN);
		delay(1000000);
		gpio_clear(LED_PORT, LED_PIN);
	} else {
		/* If there was at least one sensor detected, blink shortly for
		 * every sensor. */
		for (size_t i = 0; i < sensor_count; i++) {
			gpio_set(LED_PORT, LED_PIN);
			delay(30000);
			gpio_clear(LED_PORT, LED_PIN);
			delay(50000);
		}
	}
	delay(1000000);
	periodic_timer_setup();

	while (1) {
		/* This piece of code runs every time the MCU is woken. Reset the watchdog here.
		 * If we are being stuck in the interrupt handler (sensor reading) or anything goes
		 * wrong, the watchdog resets the MCU. */
		iwdg_reset();
		/* Enter the low power stop mode. */
		// PWR_CR |= PWR_CR_LPSDSR;
		PWR_CR &= ~PWR_CR_PDDS;
		// PWR_CR &= ~PWR_CR_LPDS;
		// SCB_SCR |= SCB_SCR_SLEEPDEEP;
		__asm volatile ("wfi");
	}

	return 0;
}


