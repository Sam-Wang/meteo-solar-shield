#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct {
	uint32_t i2c;
	uint8_t addr;

} Mpl3115;


sensor_ret_t mpl3115_detect(uint32_t i2c, uint8_t addr);
sensor_ret_t mpl3115_init(Mpl3115 *self, uint32_t i2c, uint8_t addr);
sensor_ret_t mpl3115_measure_pressure(Mpl3115 *self, float *pressure_pa, float *temperature_c);
