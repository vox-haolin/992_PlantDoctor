#include "gui.h"
#include <string.h>
#include <stdio.h>
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"

static uint32_t status_bar_color[3] = {
    0xFF00FF00,
    0xFFFFFF00,
    0xFFFF0000
};

static const char *alarm_text[3] = {
    "Normal",
    "Warning",
    "Alarm"
};

void gui_init(void)
{
    UTIL_LCD_SetLayer(1);
    UTIL_LCD_Clear(0x00000000);
    UTIL_LCD_SetFont(&Font24);
    UTIL_LCD_SetBackColor(0x40000000);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
}

void gui_update_status_bar(AlarmLevel level)
{
    UTIL_LCD_SetLayer(1);
    UTIL_LCD_SetFont(&Font20);
    UTIL_LCD_SetBackColor(status_bar_color[level]);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
    UTIL_LCD_FillRect(0, 0, LCD_FG_WIDTH, GUI_STATUS_BAR_H, status_bar_color[level]);
    UTIL_LCD_DisplayStringAt(10, 5, (uint8_t *)alarm_text[level], CENTER_MODE);
    UTIL_LCD_DisplayStringAt(LCD_FG_WIDTH - 100, 5, (uint8_t *)"SmartPlantDoctor v1.0", RIGHT_MODE);
}

void gui_update_sensor_panel(const BME280_Data *bme, const SoilMoisture_Data *soil, const RainSensor_Data *rain)
{
    char buf[64];
    UTIL_LCD_SetLayer(1);
    UTIL_LCD_SetFont(&Font16);
    UTIL_LCD_SetBackColor(0x80000000);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);

    int y = GUI_STATUS_BAR_H + 5;
    int x = 5;

    if (bme) {
        snprintf(buf, sizeof(buf), "Temp: %d.%d C", bme->temperature / 100, bme->temperature % 100);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
        y += 20;
        snprintf(buf, sizeof(buf), "Hum: %u.%u%%", bme->humidity / 1024, (bme->humidity % 1024) * 100 / 1024);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
        y += 20;
        snprintf(buf, sizeof(buf), "Press: %u hPa", bme->pressure / 100);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
        y += 20;
    }

    if (soil) {
        snprintf(buf, sizeof(buf), "Soil: %u%% (ADC:%u)", soil->percent, soil->raw_adc);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
        y += 20;
    }

    if (rain) {
        const char *rlevel[] = {"None", "Light", "Moderate", "Heavy"};
        uint8_t rl = rain->level;
        if (rl > 3) rl = 3;
        snprintf(buf, sizeof(buf), "Rain: %s", rlevel[rl]);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
        y += 20;
    }
}

void gui_update_ai_panel(const char *class_name, float confidence, const DecisionResult *result)
{
    char buf[128];
    UTIL_LCD_SetLayer(1);
    UTIL_LCD_SetFont(&Font20);
    UTIL_LCD_SetBackColor(0x80000000);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);

    int y = LCD_FG_HEIGHT - GUI_AI_PANEL_H + 5;
    int x = 5;

    snprintf(buf, sizeof(buf), "AI: %s %.0f%%", class_name, confidence * 100);
    UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
    y += 25;

    if (result) {
        snprintf(buf, sizeof(buf), "%s", result->message);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
        y += 25;
        snprintf(buf, sizeof(buf), "%s", result->suggestion);
        UTIL_LCD_DisplayStringAt(x, y, (uint8_t *)buf, LEFT_MODE);
    }
}

void gui_commit(void)
{
}
