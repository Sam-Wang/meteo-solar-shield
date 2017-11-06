#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/exti.h>

#define LED_PORT GPIOB
#define LED_PIN GPIO0

#define I2C_TIMEOUT 100000

extern void initialise_monitor_handles(void);


static void clock_setup(void) {
	// rcc_set_hpre(RCC_CFGR_HPRE_DIV16);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_I2C1);
}


static void gpio_setup(void) {
	/* Initialize the STAT LED. */
	gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);

	/* Initialize the I2C bus. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
	gpio_set_af(GPIOB, GPIO_AF1, GPIO6 | GPIO7);
}


static void i2c_setup(void) {
	i2c_reset(I2C1);
	i2c_peripheral_disable(I2C1);
	i2c_enable_analog_filter(I2C1);
	i2c_set_digital_filter(I2C1, I2C_CR1_DNF_DISABLED);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, 8);
	i2c_enable_stretching(I2C1);
	i2c_set_7bit_addr_mode(I2C1);
	i2c_peripheral_enable(I2C1);
}


static int32_t si7021_temp(void) {
	uint8_t rh[2] = {0, 0};
	i2c_transfer7(I2C1, 0x40, &(uint8_t){0xe5}, 1, rh, 2);
	uint8_t temp[2] = {0, 0};
	i2c_transfer7(I2C1, 0x40, &(uint8_t){0xe0}, 1, temp, 2);

	return ((((temp[0] << 8) | temp[1]) * 17572) >> 16) - 4685;
}


static int32_t si7021_rh(void) {
	uint8_t rh[2] = {0, 0};
	i2c_transfer7(I2C1, 0x40, &(uint8_t){0xe5}, 1, rh, 2);
	uint8_t temp[2] = {0, 0};
	i2c_transfer7(I2C1, 0x40, &(uint8_t){0xe0}, 1, temp, 2);

	return ((((rh[0] << 8) | rh[1]) * 12500) >> 16) - 600;
}


static int32_t mpl3115_pressure(void) {
	uint8_t pres[3];
	uint8_t coeff[8];
	uint8_t status;
	i2c_transfer7(I2C1, 0x60, &(uint8_t){0x13, 0x07}, 2, NULL, 0);
	i2c_transfer7(I2C1, 0x60, &(uint8_t){0x26, 0x35}, 2, NULL, 0);
	i2c_transfer7(I2C1, 0x60, &(uint8_t){0x01}, 1, pres, 3);
	return ((pres[0] << 16) | (pres[1] << 8) | (pres[2])) >> 4;
}

static int32_t mpl3115_temp(void) {
	int8_t temp_int;
	uint8_t temp_frac;
	i2c_transfer7(I2C1, 0x60, &(uint8_t){0x04}, 1, &temp_int, 1);
	i2c_transfer7(I2C1, 0x60, &(uint8_t){0x05}, 1, &temp_frac, 1);

	/* muh */
	int16_t temp = ((temp_int << 8) | temp_frac) >> 4;
	return temp * 1000 / 16;

}



int main(void) {

	clock_setup();
	initialise_monitor_handles();
	gpio_setup();
	i2c_setup();

	while (1) {
		gpio_toggle(LED_PORT, LED_PIN);
		for (uint32_t i = 0; i < 100000; i++) {
			__asm__("nop");
		}
		printf("temp=%d rh=%d pres=%d temp2=%d\n", si7021_temp(), si7021_rh(), mpl3115_pressure(), mpl3115_temp());
	}

	return 0;
}


