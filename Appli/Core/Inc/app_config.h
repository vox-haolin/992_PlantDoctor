/**
 ****************************************************************************************************
 * @file        app_config.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-13
 * @brief       app_config.h文件
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

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include "stm32n6xx_hal.h"

#define LCD_BG_WIDTH                    480
#define LCD_BG_HEIGHT                   480
#define LCD_FG_WIDTH                    800
#define LCD_FG_HEIGHT                   480

#define CAMERA_MIRROR_FLIP              CMW_MIRRORFLIP_MIRROR

#define NN_WIDTH                        240
#define NN_HEIGHT                       240
#define NN_FORMAT                       DCMIPP_PIXEL_PACKER_FORMAT_RGB888_YUV444_1
#define NN_BPP                          3
#define NN_CLASSES                      8
#define NN_CLASSES_TABLE                {\
                                            "健康",\
                                            "白粉病",\
                                            "锈病",\
                                            "叶斑病",\
                                            "缺氮",\
                                            "缺钾",\
                                            "病毒病",\
                                            "灰霉病"\
                                        }
#define NN_OBJDETECT_PROBE_THRESHOLD    0.85f
#define NN_OUTPUT_NUMBER                1

#endif
