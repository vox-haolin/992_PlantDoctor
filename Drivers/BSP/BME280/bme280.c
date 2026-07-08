#include "bme280.h"

extern I2C_HandleTypeDef hi2c2;

static int32_t bme280_compensate_temp(BME280_Handle *h, int32_t adc_T);
static uint32_t bme280_compensate_press(BME280_Handle *h, int32_t adc_P);
static uint32_t bme280_compensate_hum(BME280_Handle *h, int32_t adc_H);

uint8_t bme280_init(BME280_Handle *h)
{
    uint8_t id;

    HAL_I2C_Mem_Read(&hi2c2, BME280_ADDR, BME280_REG_ID, I2C_MEMADD_SIZE_8BIT, &id, 1, 100);
    if (id != BME280_CHIP_ID) return 1;

    /* Read calibration data */
    uint8_t calib[26];
    HAL_I2C_Mem_Read(&hi2c2, BME280_ADDR, 0x88, I2C_MEMADD_SIZE_8BIT, calib, 26, 100);
    h->calib.dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    h->calib.dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    h->calib.dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);
    h->calib.dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    h->calib.dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    h->calib.dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    h->calib.dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    h->calib.dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    h->calib.dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    h->calib.dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    h->calib.dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    h->calib.dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);

    uint8_t calib2[8];
    HAL_I2C_Mem_Read(&hi2c2, BME280_ADDR, 0xA1, I2C_MEMADD_SIZE_8BIT, &calib2[0], 1, 100);
    h->calib.dig_H1 = calib2[0];
    HAL_I2C_Mem_Read(&hi2c2, BME280_ADDR, 0xE1, I2C_MEMADD_SIZE_8BIT, calib2, 7, 100);
    h->calib.dig_H2 = (int16_t)(calib2[1] << 8 | calib2[0]);
    h->calib.dig_H3 = calib2[2];
    h->calib.dig_H4 = (int16_t)((int16_t)calib2[3] << 4 | (calib2[4] & 0x0F));
    h->calib.dig_H5 = (int16_t)((int16_t)calib2[5] << 4 | (calib2[4] >> 4));
    h->calib.dig_H6 = (int8_t)calib2[6];

    /* Configure: oversampling x1 for all, normal mode */
    uint8_t cfg;
    cfg = 0x01; /* humidity oversampling x1 */
    HAL_I2C_Mem_Write(&hi2c2, BME280_ADDR, BME280_REG_CTRL_HUM, I2C_MEMADD_SIZE_8BIT, &cfg, 1, 100);
    cfg = 0x27; /* temp oversampling x1, press oversampling x1, normal mode */
    HAL_I2C_Mem_Write(&hi2c2, BME280_ADDR, BME280_REG_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &cfg, 1, 100);
    cfg = 0xA0; /* t_sb=0.5ms, filter off */
    HAL_I2C_Mem_Write(&hi2c2, BME280_ADDR, BME280_REG_CONFIG, I2C_MEMADD_SIZE_8BIT, &cfg, 1, 100);

    HAL_Delay(10);
    return 0;
}

void bme280_read_all(BME280_Handle *h)
{
    uint8_t data[8];
    HAL_I2C_Mem_Read(&hi2c2, BME280_ADDR, BME280_REG_PRESS_MSB, I2C_MEMADD_SIZE_8BIT, data, 8, 100);

    int32_t adc_P = (int32_t)((uint32_t)data[0] << 12 | (uint32_t)data[1] << 4 | (uint32_t)data[2] >> 4);
    int32_t adc_T = (int32_t)((uint32_t)data[3] << 12 | (uint32_t)data[4] << 4 | (uint32_t)data[5] >> 4);
    int32_t adc_H = (int32_t)((uint32_t)data[6] << 8 | (uint32_t)data[7]);

    h->temperature = bme280_compensate_temp(h, adc_T);
    h->pressure = bme280_compensate_press(h, adc_P);
    h->humidity = bme280_compensate_hum(h, adc_H);
}

static int32_t t_fine;

static int32_t bme280_compensate_temp(BME280_Handle *h, int32_t adc_T)
{
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)h->calib.dig_T1 << 1))) * ((int32_t)h->calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)h->calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)h->calib.dig_T1))) >> 12) * ((int32_t)h->calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

static uint32_t bme280_compensate_press(BME280_Handle *h, int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)h->calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)h->calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)h->calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)h->calib.dig_P3) >> 8) + ((var1 * (int64_t)h->calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)h->calib.dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)h->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)h->calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)h->calib.dig_P7) << 4);
    return (uint32_t)(p >> 8);
}

static uint32_t bme280_compensate_hum(BME280_Handle *h, int32_t adc_H)
{
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)h->calib.dig_H4) << 20) - (((int32_t)h->calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)h->calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)h->calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)h->calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)h->calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    return (uint32_t)(v_x1_u32r >> 12);
}
