/**
 * @file key_driver.h
 * @brief 按键驱动头文件，状态机实现的按键检测
 */

#ifndef __KEY_DRIVE_H__
#define __KEY_DRIVE_H__

#include "main.h"

/*=======================================*/
#define KEY1_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_0

/*=======================================*/



/**
 * @brief 按键状态枚举
 */
typedef enum
{
    KS_RELEASE,        // 按键释放状态
    KS_PRESS_SHAKE,    // 按键按下抖动检测
    KS_PRESS,          // 按键稳定按下
    KS_RELEASE_SHAKE,  // 按键释放抖动检测
} KEY_STATUS;

/**
 * @brief 引脚条件枚举
 */
typedef enum
{
    COND_LOW,          // 检测到低电平
    COND_HIGH          // 检测到高电平
} KEY_CONDITION;

/**
 * @brief 按键结构体定义
 */
typedef struct
{
    GPIO_TypeDef* port;    // GPIO端口
    uint16_t pin;          // GPIO引脚
    uint8_t pin_state;     // 当前引脚状态 0/1
    KEY_STATUS status;     // 当前按键状态
    uint8_t isPressed;     // 当前按键是否处于按下状态
    void (*press_callback)(void);   // 按下回调函数
    void (*release_callback)(void); // 释放回调函数
} KeyTypedef;

/**
 * @brief 状态转换结构体
 */
typedef struct 
{
    KEY_STATUS current_status;   // 当前按键状态
    KEY_CONDITION condition;     // 触发条件
    KEY_STATUS next_status;      // 下一个按键状态
    void (*action)(KeyTypedef*); // 执行动作函数
} StateTransition;

/**
 * @brief 按键驱动初始化
 * @param timer_period 定时器周期(ms)
 * @return 无
 */
void KEY_Init(void);

/**
 * @brief 注册一个新按键
 * @param port GPIO端口
 * @param pin GPIO引脚号
 * @param press_callback 按下回调函数，可为NULL
 * @param release_callback 释放回调函数，可为NULL
 * @return 按键索引，失败返回-1
 */
int8_t KEY_Register(GPIO_TypeDef* port, uint16_t pin, void (*press_callback)(void), void (*release_callback)(void));

/**
 * @brief 按键状态检测，在定时器中断中调用
 * @return 无
 */
void KEY_ScanInTick(void);

/**
 * @brief 获取按键状态
 * @param key_index 按键索引
 * @return 1表示按下，0表示释放
 */
uint8_t KEY_IsPressed(uint8_t key_index);

#endif  

