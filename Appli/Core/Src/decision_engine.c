#include "decision_engine.h"
#include <string.h>
#include <stdio.h>

void decision_engine_init(void)
{
}

void decision_engine_process(const AIResult *ai, const EnvData *env, DecisionResult *result)
{
    result->level = ALARM_GREEN;
    strcpy(result->message, "状态正常");
    strcpy(result->suggestion, "无需处理");

    uint8_t is_healthy = (strcmp(ai->class_name, "健康") == 0);

    if (is_healthy)
    {
        if (env->soil_moisture < 20)
        {
            result->level = ALARM_YELLOW;
            sprintf(result->message, "土壤过干: %u%%", env->soil_moisture);
            sprintf(result->suggestion, "建议浇水");
        }
        else if (env->soil_moisture > 80)
        {
            result->level = ALARM_YELLOW;
            sprintf(result->message, "土壤过湿: %u%%", env->soil_moisture);
            sprintf(result->suggestion, "建议排水降湿");
        }
        else if (env->temperature > 3500)
        {
            result->level = ALARM_YELLOW;
            sprintf(result->message, "温度过高: %d.%d°C", env->temperature / 100, env->temperature % 100);
            sprintf(result->suggestion, "建议降温通风");
        }
        else
        {
            result->level = ALARM_GREEN;
            strcpy(result->message, "状态正常");
            strcpy(result->suggestion, "无需处理");
        }
        return;
    }

    if (ai->confidence < 0.7f)
    {
        result->level = ALARM_YELLOW;
        strcpy(result->message, "疑似病害，置信度不足");
        strcpy(result->suggestion, "建议重新拍摄确认");
        return;
    }

    if (strstr(ai->class_name, "白粉病"))
    {
        if (env->temperature > 2800)
        {
            result->level = ALARM_RED;
            sprintf(result->message, "高温诱发白粉病 (%.0f%%)", ai->confidence * 100);
            sprintf(result->suggestion, "建议降温通风，喷洒杀菌剂");
        }
        else
        {
            result->level = ALARM_RED;
            sprintf(result->message, "检测到白粉病 (%.0f%%)", ai->confidence * 100);
            sprintf(result->suggestion, "建议喷洒硫磺悬浮剂或三唑酮");
        }
    }
    else if (strstr(ai->class_name, "锈病"))
    {
        result->level = ALARM_RED;
        sprintf(result->message, "检测到锈病 (%.0f%%)", ai->confidence * 100);
        sprintf(result->suggestion, "建议清除病叶，喷洒三唑酮或代森锰锌");
    }
    else if (strstr(ai->class_name, "叶斑病"))
    {
        if (env->soil_moisture > 75)
        {
            result->level = ALARM_RED;
            sprintf(result->message, "高湿诱发叶斑病 (%.0f%%)", ai->confidence * 100);
            sprintf(result->suggestion, "建议排水降湿，喷洒百菌清或苯醚甲环唑");
        }
        else
        {
            result->level = ALARM_RED;
            sprintf(result->message, "检测到叶斑病 (%.0f%%)", ai->confidence * 100);
            sprintf(result->suggestion, "建议喷洒百菌清或多菌灵");
        }
    }
    else if (strstr(ai->class_name, "缺氮"))
    {
        result->level = ALARM_YELLOW;
        sprintf(result->message, "检测到缺氮症状 (%.0f%%)", ai->confidence * 100);
        sprintf(result->suggestion, "建议追施氮肥，如尿素或复合肥");
    }
    else if (strstr(ai->class_name, "缺钾"))
    {
        result->level = ALARM_YELLOW;
        sprintf(result->message, "检测到缺钾症状 (%.0f%%)", ai->confidence * 100);
        sprintf(result->suggestion, "建议追施钾肥，如硫酸钾或磷酸二氢钾");
    }
    else
    {
        result->level = ALARM_RED;
        sprintf(result->message, "检测到%s (%.0f%%)", ai->class_name, ai->confidence * 100);
        sprintf(result->suggestion, "建议咨询农业专家");
    }
}
