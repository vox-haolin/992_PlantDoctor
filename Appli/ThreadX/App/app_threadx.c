#include "app_threadx.h"
#include "app_config.h"
#include "app_lcd.h"
#include "app_camera.h"
#include "sensors.h"
#include "decision_engine.h"
#include "gui.h"
#include "sd_log.h"
#include "uart3.h"
#include "bme280.h"
#include "ll_aton_runtime.h"
#include "bsp_lcd.h"
#include <string.h>
#include <stdio.h>

/* Priority: lower number = higher priority (ThreadX convention) */
#define PRIO_WATCHDOG       5
#define PRIO_AI_INFERENCE   6
#define PRIO_DECISION       7
#define PRIO_CAMERA         7
#define PRIO_GUI            8
#define PRIO_ESP32          8
#define PRIO_SENSOR         9
#define PRIO_SD_LOG         10

#define STACK_AI            4096
#define STACK_CAMERA        2048
#define STACK_DECISION      2048
#define STACK_GUI           4096
#define STACK_ESP32         2048
#define STACK_SENSOR        1024
#define STACK_SD_LOG        2048
#define STACK_WATCHDOG      512

static TX_THREAD thread_ai;
static TX_THREAD thread_camera;
static TX_THREAD thread_decision;
static TX_THREAD thread_gui;
static TX_THREAD thread_esp32;
static TX_THREAD thread_sensor;
static TX_THREAD thread_sd;
static TX_THREAD thread_watchdog;

TX_QUEUE queue_ai_result;
TX_QUEUE queue_sensor_data;
TX_QUEUE queue_decision_result;

static void ai_thread_entry(ULONG id);
static void camera_thread_entry(ULONG id);
static void decision_thread_entry(ULONG id);
static void gui_thread_entry(ULONG id);
static void esp32_thread_entry(ULONG id);
static void sensor_thread_entry(ULONG id);
static void sd_log_thread_entry(ULONG id);
static void watchdog_thread_entry(ULONG id);

extern const char *nn_classes_table[NN_CLASSES];
extern volatile uint32_t app_nn_frame_count;
extern void app_camera_nn_pipe_frame_cb(void);

UINT app_threadx_init(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;
    CHAR *pointer;

    tx_queue_create(&queue_ai_result, "AI Result", TX_2_ULONG, NULL, 16 * sizeof(ULONG));
    tx_queue_create(&queue_sensor_data, "Sensor Data", TX_2_ULONG, NULL, 16 * sizeof(ULONG));
    tx_queue_create(&queue_decision_result, "Decision Result", TX_2_ULONG, NULL, 16 * sizeof(ULONG));

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_SENSOR, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_sensor, "Sensor", sensor_thread_entry, 0, pointer, STACK_SENSOR, PRIO_SENSOR, PRIO_SENSOR, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_CAMERA, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_camera, "Camera", camera_thread_entry, 0, pointer, STACK_CAMERA, PRIO_CAMERA, PRIO_CAMERA, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_AI, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_ai, "AI Inference", ai_thread_entry, 0, pointer, STACK_AI, PRIO_AI_INFERENCE, PRIO_AI_INFERENCE, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_DECISION, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_decision, "Decision", decision_thread_entry, 0, pointer, STACK_DECISION, PRIO_DECISION, PRIO_DECISION, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_GUI, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_gui, "GUI", gui_thread_entry, 0, pointer, STACK_GUI, PRIO_GUI, PRIO_GUI, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_ESP32, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_esp32, "ESP32", esp32_thread_entry, 0, pointer, STACK_ESP32, PRIO_ESP32, PRIO_ESP32, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_SD_LOG, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_sd, "SD Log", sd_log_thread_entry, 0, pointer, STACK_SD_LOG, PRIO_SD_LOG, PRIO_SD_LOG, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (tx_byte_allocate(byte_pool, (VOID **)&pointer, STACK_WATCHDOG, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
    tx_thread_create(&thread_watchdog, "Watchdog", watchdog_thread_entry, 0, pointer, STACK_WATCHDOG, PRIO_WATCHDOG, PRIO_WATCHDOG, TX_NO_TIME_SLICE, TX_AUTO_START);

    return TX_SUCCESS;
}

static void watchdog_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    while (1)
    {
        tx_thread_sleep(100);
    }
}

static void sensor_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    BME280_Data bme;
    SoilMoisture_Data soil;
    RainSensor_Data rain;

    sensors_init();

    while (1)
    {
        sensors_read_all(&bme, &soil, &rain);
        tx_thread_sleep(100);
    }
}

static void camera_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    while (1)
    {
        tx_thread_sleep(1);
    }
}

static void ai_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    uint32_t index;
    uint32_t time_stamp[2];
    float_t *nn_out[NN_OUTPUT_NUMBER];
    uint32_t nn_out_len[NN_OUTPUT_NUMBER];
    uint8_t *nn_in;

    LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(Default);
    const LL_Buffer_InfoTypeDef *nn_in_info = LL_ATON_Input_Buffers_Info_Default();
    const LL_Buffer_InfoTypeDef *nn_out_info = LL_ATON_Output_Buffers_Info_Default();
    nn_in = (uint8_t *)LL_Buffer_addr_start(&nn_in_info[0]);

    for (uint8_t i = 0; i < NN_OUTPUT_NUMBER; i++)
    {
        nn_out[i] = (float_t *)LL_Buffer_addr_start(&nn_out_info[i]);
        nn_out_len[i] = LL_Buffer_len(&nn_out_info[i]);
    }

    while (1)
    {
        app_camera_isp_update();
        app_camera_nn_pipe_start(nn_in, CMW_MODE_SNAPSHOT);
        while (app_nn_frame_count == 0) tx_thread_sleep(1);
        app_nn_frame_count = 0;

        time_stamp[0] = HAL_GetTick();
        LL_ATON_RT_Main(&NN_Instance_Default);
        time_stamp[1] = HAL_GetTick();

        index = app_get_array_max_index(nn_out[0], nn_out_len[0]);

        for (uint8_t i = 0; i < NN_OUTPUT_NUMBER; i++)
        {
            SCB_InvalidateDCache_by_Addr(nn_out[i], nn_out_len[i]);
        }

        ULONG msg[2];
        msg[0] = index;
        msg[1] = time_stamp[1] - time_stamp[0];
        tx_queue_send(&queue_ai_result, msg, TX_WAIT_FOREVER);
    }
}

static void decision_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    AIResult ai_res;
    EnvData env;
    DecisionResult dec_res;

    decision_engine_init();

    while (1)
    {
        ULONG msg[2];
        tx_queue_receive(&queue_ai_result, msg, TX_WAIT_FOREVER);
        ai_res.class_index = msg[0];
        ai_res.confidence = 0.9f;

        if (ai_res.class_index < NN_CLASSES)
        {
            strncpy(ai_res.class_name, nn_classes_table[ai_res.class_index], sizeof(ai_res.class_name) - 1);
            ai_res.class_name[sizeof(ai_res.class_name) - 1] = 0;
        }
        else
        {
            strcpy(ai_res.class_name, "Unknown");
        }

        env.temperature = 2500;
        env.humidity = 50000;
        env.soil_moisture = 50;
        env.rain_level = 0;

        decision_engine_process(&ai_res, &env, &dec_res);

        ULONG dec_msg[4];
        dec_msg[0] = (ULONG)dec_res.level;
        tx_queue_send(&queue_decision_result, dec_msg, TX_NO_WAIT);
    }
}

static void gui_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);

    app_lcd_init();
    app_camera_init(NULL, NULL, NULL, app_camera_nn_pipe_frame_cb);
    app_camera_display_pipe_start(app_lcd_get_bg_buffer(), CMW_MODE_CONTINUOUS);
    gui_init();

    while (1)
    {
        ULONG msg[4];
        if (tx_queue_receive(&queue_decision_result, msg, TX_NO_WAIT) == TX_SUCCESS)
        {
            gui_update_status_bar((AlarmLevel)msg[0]);
        }

        BME280_Data bme = {2500, 50000, 101325};
        SoilMoisture_Data soil = {1500, 50};
        RainSensor_Data rain = {0, 0};
        gui_update_sensor_panel(&bme, &soil, &rain);
        gui_update_ai_panel("Healthy", 0.95f, NULL);
        gui_commit();

        tx_thread_sleep(50);
    }
}

static void esp32_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    uart3_init(921600);

    while (1)
    {
        ULONG msg[4];
        if (tx_queue_receive(&queue_decision_result, msg, TX_NO_WAIT) == TX_SUCCESS)
        {
            uart3_printf("{\"alarm\":%lu}\r\n", msg[0]);
        }
        tx_thread_sleep(500);
    }
}

static void sd_log_thread_entry(ULONG id)
{
    TX_PARAMETER_NOT_USED(id);
    sd_log_init();

    while (1)
    {
        tx_thread_sleep(1000);
    }
}
