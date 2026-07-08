/**
 ****************************************************************************************************
 * @file        app.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-13
 * @brief       app.c文件
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 N647开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 * 
 ****************************************************************************************************
 */

#include "app.h"
#include "app_config.h"
#include "app_lcd.h"
#include "app_camera.h"
#include "ll_aton_runtime.h"

const char *nn_classes_table[NN_CLASSES] = NN_CLASSES_TABLE;
volatile uint32_t app_nn_frame_count = 0;

void app_camera_nn_pipe_frame_cb(void);
static uint32_t app_get_array_max_index(float_t *nn_out, uint32_t nn_out_len);
static void app_display_network_output(const char *class_name, float_t class_probe, uint32_t inference_ms);

void app_run(void)
{
    LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(Default);
    const LL_Buffer_InfoTypeDef *nn_in_info = LL_ATON_Input_Buffers_Info_Default();
    const LL_Buffer_InfoTypeDef *nn_out_info = LL_ATON_Output_Buffers_Info_Default();
    uint8_t *nn_in;
    float_t *nn_out[NN_OUTPUT_NUMBER];
    uint32_t nn_out_len[NN_OUTPUT_NUMBER];
    uint8_t i;
    uint32_t time_stamp[2];
    uint32_t index;

    nn_in = (uint8_t *)LL_Buffer_addr_start(&nn_in_info[0]);

    for (i = 0; i < NN_OUTPUT_NUMBER; i++)
    {
        nn_out[i] = (float_t *)LL_Buffer_addr_start(&nn_out_info[i]);
        nn_out_len[i] = LL_Buffer_len(&nn_out_info[i]);
    }

    app_lcd_init();
    app_camera_init(NULL, NULL, NULL, app_camera_nn_pipe_frame_cb);

    app_camera_display_pipe_start(app_lcd_get_bg_buffer(), CMW_MODE_CONTINUOUS);

    while (1)
    {
        app_camera_isp_update();

        app_camera_nn_pipe_start(nn_in, CMW_MODE_SNAPSHOT);
        while (app_nn_frame_count == 0);
        app_nn_frame_count = 0;

        time_stamp[0] = HAL_GetTick();
        LL_ATON_RT_Main(&NN_Instance_Default);
        time_stamp[1] = HAL_GetTick();

        index = app_get_array_max_index(nn_out[0], nn_out_len[0]);
        app_display_network_output(nn_classes_table[index], nn_out[0][index], time_stamp[1] - time_stamp[0]);

        for (i = 0; i < NN_OUTPUT_NUMBER; i++)
        {
            SCB_InvalidateDCache_by_Addr(nn_out[i], nn_out_len[i]);
        }
    }
}

void app_camera_nn_pipe_frame_cb(void)
{
    app_nn_frame_count++;
}

uint32_t app_get_array_max_index(float_t *nn_out, uint32_t nn_out_len)
{
    uint32_t index = 0;
    uint32_t i;
    float_t max_val = nn_out[0];

    for (i = 1; i < nn_out_len; i++)
    {
        if (nn_out[i] > max_val)
        {
            max_val = nn_out[i];
            index = i;
        }
    }

    return index;
}

static void app_display_network_output(const char *class_name, float_t class_probe, uint32_t inference_ms)
{
    uint8_t line_nb = 0;

    app_lcd_draw_area_update();

    UTIL_LCD_FillRect(0, 0, LCD_FG_WIDTH, LCD_FG_HEIGHT, 0x00000000);

    UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "Inference");
    line_nb += 1;
    UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "%ums", inference_ms);
    if (class_probe > NN_OBJDETECT_PROBE_THRESHOLD)
    {
        line_nb += 2;
        UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "%s", class_name);
        line_nb += 1;
        UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "%.0f%%", class_probe * 100);
    }

    app_lcd_draw_area_commit();
}
