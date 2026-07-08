#ifndef __SENSORS_H
#define __SENSORS_H

#include "main.h"

#define SOIL_MOISTURE_CH ADC_CHANNEL_3
#define RAIN_SENSOR_CH   ADC_CHANNEL_4

#define SOIL_DRY_ADC     900
#define SOIL_WET_ADC     2800

typedef struct {
    int32_t temperature;
    uint32_t humidity;
    uint32_t pressure;
} BME280_Data;

typedef struct {
    uint32_t raw_adc;
    uint8_t percent;
} SoilMoisture_Data;

typedef struct {
    uint32_t raw_adc;
    uint8_t level;
} RainSensor_Data;

void sensors_init(void);
void sensors_read_all(BME280_Data *bme, SoilMoisture_Data *soil, RainSensor_Data *rain);

#endif
