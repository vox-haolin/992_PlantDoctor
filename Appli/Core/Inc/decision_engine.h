#ifndef __DECISION_ENGINE_H
#define __DECISION_ENGINE_H

#include "main.h"

typedef enum {
    ALARM_GREEN = 0,
    ALARM_YELLOW,
    ALARM_RED
} AlarmLevel;

typedef struct {
    int32_t temperature;
    uint32_t humidity;
    uint32_t soil_moisture;
    uint8_t rain_level;
} EnvData;

typedef struct {
    uint32_t class_index;
    float confidence;
    char class_name[32];
} AIResult;

typedef struct {
    AlarmLevel level;
    char message[128];
    char suggestion[128];
} DecisionResult;

void decision_engine_init(void);
void decision_engine_process(const AIResult *ai, const EnvData *env, DecisionResult *result);

#endif
