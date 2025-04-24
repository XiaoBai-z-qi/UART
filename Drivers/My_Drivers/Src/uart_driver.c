#include "uart_driver.h"
#include <stdio.h>

#define SET_BAUDRATE 115200
#define DEBUG_UART_TIMEOUT 500

UART_HandleTypeDef huart1;



// 初始化UART驱动
void UART_Init(void)
{
    huart1.Instance = USART1;                           // 配置串口1的硬件实例
    huart1.Init.BaudRate = SET_BAUDRATE;                // 配置波特率
    huart1.Init.WordLength = UART_WORDLENGTH_8B;        // 配置数据位长度为8位
    huart1.Init.StopBits = UART_STOPBITS_1;             // 配置停止位为1位
    huart1.Init.Parity = UART_PARITY_NONE;              // 配置无奇偶校验
    huart1.Init.Mode = UART_MODE_TX_RX;                 // 配置为收发模式
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;        // 配置无硬件流控
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;    // 配置过采样率为16
    
    if (HAL_UART_Init(&huart1)!= HAL_OK)                // 初始化串口
    {
        Error_Handler();                                // 初始化失败，调用错误处理函数
    }
}

/**
 * @brief 重定向C库函数printf到UART
 * 
 * 该函数重写了标准C库的fputc函数，使printf等函数的输出重定向到UART
 * 
 * @param ch 要发送的字符
 * @param f 文件指针（此处未使用）
 * @return 返回发送的字符
 */
int fputc(int ch, FILE *f)
{
    (void)f;                                            // 避免未使用参数警告
    HAL_UART_Transmit(&huart1, (const uint8_t *)&ch, 1, DEBUG_UART_TIMEOUT);  // 通过UART发送单个字符
    return ch;
}


/**
 * @brief UART MSP初始化回调函数
 * 
 * 该函数是HAL库中的弱定义函数，在此处重写以配置UART相关的GPIO引脚
 * 
 * @param huart UART句柄指针
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)    //hal_uart里的弱定义
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};         // 定义GPIO初始化结构体
    if(huart -> Instance == USART1)                 // 判断是否为USART1实例
    {
        __HAL_RCC_USART1_CLK_ENABLE();              // 使能USART1时钟
        __HAL_RCC_GPIOA_CLK_ENABLE();               // 使能GPIOA时钟

        
        GPIO_InitStruct.Pin = GPIO_PIN_9;           // 配置PA9引脚(TX)
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;     // 设置为复用推挽输出模式
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // 设置为高速模式
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);     // 初始化PA9引脚

        GPIO_InitStruct.Pin = GPIO_PIN_10;          // 配置PA10引脚(RX)
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     // 设置为输入模式
        GPIO_InitStruct.Pull = GPIO_NOPULL;         // 设置为无上下拉
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);     // 初始化PA10引脚

    }
}



/**
 * @brief 反初始化 UART 硬件抽象层
 *
 * 该函数用于反初始化指定的 UART 硬件抽象层（HAL）。
 *
 * @param uartHandle UART 硬件抽象层句柄指针
 *
 * 如果指定的 UART 实例是 USART1，则该函数会禁用 USART1 的时钟，并反初始化
 * GPIO 引脚 PA9 和 PA10。
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    // 如果uartHandle的实例是USART1
    if(uartHandle->Instance == USART1)
    {
        // 禁用USART1的时钟
        __HAL_RCC_USART1_CLK_DISABLE();
        // 禁用GPIOA端口上的PIN9和PIN10的GPIO功能
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    }
}





