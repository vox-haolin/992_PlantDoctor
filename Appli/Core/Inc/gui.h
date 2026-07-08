#ifndef __GUI_H
#define __GUI_H

#include "main.h"
#include "app_config.h"
#include "bsp_lcd.h"
#include "decision_engine.h"
#include "sensors.h"

#define GUI_STATUS_BAR_H    30
#define GUI_SENSOR_PANEL_W  200
#define GUI_AI_PANEL_H      120

void gui_init(void);
void gui_update_status_bar(AlarmLevel level);
void gui_update_sensor_panel(const BME280_Data *bme, const SoilMoisture_Data *soil, const RainSensor_Data *rain);
void gui_update_ai_panel(const char *class_name, float confidence, const DecisionResult *result);
void gui_commit(void);

#endif
