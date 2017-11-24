#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "can.h"


void can_setup(void) {
	rcc_periph_clock_enable(RCC_SYSCFG_COMP);
	/* Remap CANRX/CANTX on PA11/PA12 pins. */
	SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;

	/* CANRX */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO11);
	/* CANTX */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO12);
	gpio_set_af(GPIOA, GPIO_AF4, GPIO11 | GPIO12);

	rcc_periph_clock_enable(RCC_CAN1);
	can_reset(CAN1);
	can_init(CAN1,
		false,
		true,
		false,
		true,
		false,
		false,
		CAN_BTR_SJW_1TQ,
		CAN_BTR_TS1_9TQ,
		CAN_BTR_TS2_6TQ,
		1,
		false,
		false
	);

	can_filter_id_mask_32bit_init(CAN1,
		0,     /* Filter ID */
		0,     /* CAN ID */
		0,     /* CAN ID mask */
		0,     /* FIFO */
		true
	);
}
