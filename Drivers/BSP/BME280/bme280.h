#ifndef __BME280_H
#define __BME280_H

#include "main.h"

#define BME280_ADDR         (0x76 << 1)
#define BME280_CHIP_ID      0x60

/* BME280 registers */
#define BME280_REG_ID       0xD0
#define BME280_REG_RESET    0xE0
#define BME280_REG_CTRL_HUM 0xF2
#define BME280_REG_STATUS   0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG   0xF5
#define BME280_REG_PRESS_MSB 0xF7
#define BME280_REG_TEMP_MSB 0xFA
#define BME280_REG_HUM_MSB  0xFD

typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} BME280_CalibData;

typedef struct {
    int32_t temperature;
    uint32_t pressure;
    uint32_t humidity;
    BME280_CalibData calib;
} BME280_Handle;

uint8_t bme280_init(BME280_Handle *h);
void bme280_read_all(BME280_Handle *h);

#endif
