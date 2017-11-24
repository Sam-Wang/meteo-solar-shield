#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <libopencm3/stm32/i2c.h>

#include "sensor.h"
#include "si7021.h"

uint8_t si7021_read_id1[2] = {0xfa, 0x0f};
uint8_t si7021_read_id2[2] = {0xfc, 0xc9};
uint8_t si7021_read_fw[2] = {0x84, 0xb8};
uint8_t si7021_enable_heater[2] = {0xe6, 0x3e};
uint8_t si7021_disable_heater[2] = {0xe6, 0x3a};



sensor_ret_t si7021_detect(uint32_t i2c, uint8_t addr) {
	uint8_t sna[8];
	uint8_t snb[8];
	uint8_t fw;
	i2c_transfer7(i2c, addr, si7021_read_id1, 2, sna, 8, I2C_TIMEOUT);
	i2c_transfer7(i2c, addr, si7021_read_id2, 2, snb, 8, I2C_TIMEOUT);
	i2c_transfer7(i2c, addr, si7021_read_fw, 2, &fw, 1, I2C_TIMEOUT);

	i2c_transfer7(i2c, addr, si7021_disable_heater, 2, NULL, 0, I2C_TIMEOUT);

	/* Check for the Si7021 chip, firmware revision A20. */
	if (snb[0] == 0x15 && fw == 0x20) {
		return SENSOR_OK;
	}

	return SENSOR_FAILED;
}


sensor_ret_t si7021_init(Si7021 *self, uint32_t i2c, uint8_t addr) {
	memset(self, 0, sizeof(Si7021));
	self->i2c = i2c;
	self->addr = addr;

	return SENSOR_OK;
}


sensor_ret_t si7021_measure_temp_rh(Si7021 *self, float *temp, float *rh) {
	uint8_t r[2] = {0, 0};
	i2c_transfer7(self->i2c, self->addr, (uint8_t[]){0xe5}, 1, r, 2, I2C_TIMEOUT);
	uint8_t t[2] = {0, 0};
	i2c_transfer7(self->i2c, self->addr, (uint8_t[]){0xe0}, 1, t, 2, I2C_TIMEOUT);

	*temp = (((((t[0] << 8) | t[1]) * 17572) >> 16) - 4685) / 100.0;
	*rh = (((((r[0] << 8) | r[1]) * 12500) >> 16) - 600) / 100.0;

	return SENSOR_OK;
}

