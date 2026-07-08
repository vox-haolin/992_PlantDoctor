#ifndef __UART3_H
#define __UART3_H

#include "main.h"
#include <stdio.h>

#define UART3_REC_LEN    512

extern UART_HandleTypeDef huart3;
extern uint8_t uart3_rx_buf[UART3_REC_LEN];
extern volatile uint16_t uart3_rx_len;

void uart3_init(uint32_t baudrate);
void uart3_send(const uint8_t *data, uint16_t len);
int uart3_printf(const char *fmt, ...);

#endif
