#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <libopencm3/stm32/i2c.h>

#include "sensor.h"
#include "mpl3115.h"


sensor_ret_t mpl3115_detect(uint32_t i2c, uint8_t addr) {
	uint8_t who_am_i = 0;
	i2c_transfer7(i2c, addr, (uint8_t[]){0x0c}, 1, &who_am_i, 1, I2C_TIMEOUT);

	if (who_am_i == 0xc4) {
		return SENSOR_OK;
	}

	return SENSOR_FAILED;
}


sensor_ret_t mpl3115_init(Mpl3115 *self, uint32_t i2c, uint8_t addr) {
	memset(self, 0, sizeof(Mpl3115));
	self->i2c = i2c;
	self->addr = addr;

	return SENSOR_OK;
}


sensor_ret_t mpl3115_measure_pressure(Mpl3115 *self, float *pressure_pa, float *temperature_c) {
	uint8_t pres[3];
	int8_t temp_int;
	uint8_t temp_frac;

	/* Read the old data and initiate a new conversion on the background. */
	i2c_transfer7(self->i2c, self->addr, (uint8_t[]){0x01}, 1, pres, 3, I2C_TIMEOUT);
	i2c_transfer7(self->i2c, self->addr, (uint8_t[]){0x04}, 1, (uint8_t *)&temp_int, 1, I2C_TIMEOUT);
	i2c_transfer7(self->i2c, self->addr, (uint8_t[]){0x05}, 1, &temp_frac, 1, I2C_TIMEOUT);

	i2c_transfer7(self->i2c, self->addr, (uint8_t[]){0x26, 0x3a}, 2, NULL, 0, I2C_TIMEOUT);

	int16_t temp = ((temp_int << 8) | temp_frac) >> 4;
	*temperature_c = temp / 16.0;
	*pressure_pa = (((pres[0] << 16) | (pres[1] << 8) | (pres[2])) >> 4) / 4.0;

	return SENSOR_OK;
}

