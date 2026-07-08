#include "sensors.h"
#include "adc.h"
#include "bme280.h"

static BME280_Handle hbme;

void sensors_init(void)
{
    adc_init();
    bme280_init(&hbme);
}

static uint8_t soil_calc_percent(uint32_t raw)
{
    if (raw <= SOIL_DRY_ADC) return 0;
    if (raw >= SOIL_WET_ADC) return 100;
    return (uint8_t)((uint32_t)(raw - SOIL_DRY_ADC) * 100 / (SOIL_WET_ADC - SOIL_DRY_ADC));
}

void sensors_read_all(BME280_Data *bme, SoilMoisture_Data *soil, RainSensor_Data *rain)
{
    bme280_read_all(&hbme);
    if (bme) {
        bme->temperature = hbme.temperature;
        bme->humidity = hbme.humidity;
        bme->pressure = hbme.pressure;
    }

    uint32_t soil_adc = adc_get_result_average(SOIL_MOISTURE_CH, 10);
    if (soil) {
        soil->raw_adc = soil_adc;
        soil->percent = soil_calc_percent(soil_adc);
    }

    uint32_t rain_adc = adc_get_result_average(RAIN_SENSOR_CH, 10);
    if (rain) {
        rain->raw_adc = rain_adc;
        if (rain_adc < 500) rain->level = 0;
        else if (rain_adc < 1200) rain->level = 1;
        else if (rain_adc < 2200) rain->level = 2;
        else rain->level = 3;
    }
}
