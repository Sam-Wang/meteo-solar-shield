#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct {
	uint32_t i2c;
	uint8_t addr;

} Si7021;


sensor_ret_t si7021_detect(uint32_t i2c, uint8_t addr);
sensor_ret_t si7021_init(Si7021 *self, uint32_t i2c, uint8_t addr);
sensor_ret_t si7021_measure_temp_rh(Si7021 *self, float *temp, float *rh);
