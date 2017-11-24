#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "sensor.h"
#include "can.h"

#include "si7021.h"
#include "mpl3115.h"


struct sensor sensors[SENSOR_COUNT_MAX];
size_t sensor_count = 0;
uint32_t sensor_base_id = 200;


void sensor_setup(void) {
	/* Initialize the I2C bus. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
	gpio_set_af(GPIOB, GPIO_AF1, GPIO6 | GPIO7);

	rcc_periph_clock_enable(RCC_I2C1);

	i2c_reset(I2C1);
	i2c_peripheral_disable(I2C1);
	i2c_enable_analog_filter(I2C1);
	i2c_set_digital_filter(I2C1, I2C_CR1_DNF_DISABLED);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, 8);
	i2c_enable_stretching(I2C1);
	i2c_set_7bit_addr_mode(I2C1);
	i2c_peripheral_enable(I2C1);
}


void sensor_transmit(uint32_t sensor_id, enum sensor_value_type type, float sensor_value) {
	uint8_t data[5] = {0};

	/* Assume that the float is 4 bytes long. */
	memcpy(data, &sensor_value, 4);
	data[4] = type;
	can_transmit(CAN1, sensor_id, false, false, 5, data);
}


void sensor_measure_all(void) {
	uint32_t sensor_id = sensor_base_id;
	for (size_t i = 0; i < sensor_count; i++) {
		switch (sensors[i].type) {
			case SENSOR_TYPE_SI7021: {
				Si7021 *s = (Si7021 *)(sensors[i].sensor_instance);
				float temp = 0.0;
				float rh = 0.0;
				si7021_measure_temp_rh(s, &temp, &rh);
				sensor_transmit(sensor_id++, SENSOR_VALUE_TYPE_TEMP_C, temp);
				sensor_transmit(sensor_id++, SENSOR_VALUE_TYPE_RH_PERCENT, rh);
				#if defined(SEMIHOSTING)
					printf("Si7021 temp=%d.%02d rh=%d.%02d\n", (int32_t)temp, (int32_t)(temp * 100.0) % 100, (int32_t)rh, (int32_t)(rh * 100) % 100);
				#endif
				break;
			}

			case SENSOR_TYPE_MPL3115: {
				Mpl3115 *s = (Mpl3115 *)(sensors[i].sensor_instance);
				float pressure_pa = 0.0;
				float temperature_c = 0.0;
				mpl3115_measure_pressure(s, &pressure_pa, &temperature_c);
				sensor_transmit(sensor_id++, SENSOR_VALUE_TYPE_PRESSURE_PA, pressure_pa);
				sensor_transmit(sensor_id++, SENSOR_VALUE_TYPE_TEMP_C, temperature_c);
				#if defined(SEMIHOSTING)
					printf("Mpl3115 pressure=%d temperature=%d.%02d\n", (int32_t)pressure_pa, (int32_t)temperature_c, (int32_t)(temperature_c * 100.0) % 100);
				#endif
				break;
			}

			default:
				break;
		}
	}
}


void sensor_detect_all_buses(void) {
	sensor_count = 0;
	sensor_detect_all(I2C1);
}


void sensor_detect_all(uint32_t i2c) {
	if (si7021_detect(i2c, 0x40) == SENSOR_OK) {
		sensors[sensor_count].type = SENSOR_TYPE_SI7021;
		sensors[sensor_count].sensor_instance = malloc(sizeof(Si7021));
		si7021_init((Si7021 *)(sensors[sensor_count].sensor_instance), i2c, 0x40);
		sensor_count++;
	}
	if (mpl3115_detect(i2c, 0x60) == SENSOR_OK) {
		sensors[sensor_count].type = SENSOR_TYPE_MPL3115;
		sensors[sensor_count].sensor_instance = malloc(sizeof(Mpl3115));
		mpl3115_init((Mpl3115 *)(sensors[sensor_count].sensor_instance), i2c, 0x60);
		sensor_count++;
	}
}
