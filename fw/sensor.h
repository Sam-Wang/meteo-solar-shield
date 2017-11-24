#pragma once

#include <stdint.h>
#include <stdbool.h>


#define SENSOR_COUNT_MAX 16
#define I2C_TIMEOUT 10000

typedef enum {
	SENSOR_OK = 0,
	SENSOR_FAILED = 1,
} sensor_ret_t;

enum sensor_value_type {
	SENSOR_VALUE_TYPE_NONE = 0,
	SENSOR_VALUE_TYPE_TEMP_C = 1,
	SENSOR_VALUE_TYPE_PRESSURE_PA = 2,
	SENSOR_VALUE_TYPE_RH_PERCENT = 3,
};

enum sensor_type {
	SENSOR_TYPE_SI7021,
	SENSOR_TYPE_MPL3115,
};

struct sensor {
	enum sensor_type type;
	void *sensor_instance;
};


extern size_t sensor_count;
extern uint32_t sensor_base_id;

void sensor_setup(void);
void sensor_transmit(uint32_t sensor_id, enum sensor_value_type type, float sensor_value);
void sensor_measure_all(void);
void sensor_detect_all_buses(void);
void sensor_detect_all(uint32_t i2c);
