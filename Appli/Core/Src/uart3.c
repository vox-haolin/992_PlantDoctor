#include "uart3.h"
#include <stdarg.h>
#include <string.h>

UART_HandleTypeDef huart3;
uint8_t uart3_rx_buf[UART3_REC_LEN];
volatile uint16_t uart3_rx_len = 0;
static uint8_t uart3_rx_byte;

void uart3_init(uint32_t baudrate)
{
    __HAL_RCC_UART3_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_UART3;
    HAL_GPIO_Init(GPIOD, &gpio);

    huart3.Instance = UART3;
    huart3.Init.BaudRate = baudrate;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();

    HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1);
}

void uart3_send(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)data, len, 1000);
}

int uart3_printf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n > 0) uart3_send((uint8_t *)buf, n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1);
    return n;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART3)
    {
        if (uart3_rx_len < UART3_REC_LEN)
            uart3_rx_buf[uart3_rx_len++] = uart3_rx_byte;
        HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1);
    }
}
