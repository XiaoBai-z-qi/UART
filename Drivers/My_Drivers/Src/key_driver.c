#include "key_driver.h"

#define MAX_KEYS 16 // 最大支持的按键数量

static KeyTypedef Keys[MAX_KEYS]; // 按键数组
static uint8_t key_count = 0;     // 当前注册的按键数量


void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = KEY1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(KEY1_GPIO_Port, &GPIO_InitStruct);

    // 清空按键数组
    key_count = 0;
    // 初始化通常由用户完成，这里只是预留接口
}



// 按键动作函数
static void mark_pressed(KeyTypedef* key) {
    key->isPressed = 1;
    if (key->press_callback != NULL) {
        key->press_callback();
    }
}

static void clear_pressed(KeyTypedef* key) {
    key->isPressed = 0;
    if (key->release_callback != NULL) {
        key->release_callback();
    }
}

/* 状态转换表 */
static const StateTransition transitionTable[] = {
    // 当前状态        条件        下一状态        动作
    {KS_RELEASE,       COND_LOW,   KS_PRESS_SHAKE, NULL},
    {KS_PRESS_SHAKE,   COND_LOW,   KS_PRESS,       mark_pressed},
    {KS_PRESS_SHAKE,   COND_HIGH,  KS_RELEASE,     NULL},
    {KS_PRESS,         COND_HIGH,  KS_RELEASE_SHAKE, NULL},
    {KS_RELEASE_SHAKE, COND_HIGH,  KS_RELEASE,     clear_pressed},
    {KS_RELEASE_SHAKE, COND_LOW,   KS_PRESS,       NULL}
};

/**
 * @brief 按键状态检测
 * @param key 按键指针
 * @return 无
 */
static void key_status_check(KeyTypedef* key)
{
    // 按键引脚状态
    KEY_CONDITION current_cond = key->pin_state ? COND_HIGH : COND_LOW;

    for (uint8_t i = 0; i < sizeof(transitionTable)/sizeof(StateTransition); i++) {
        const StateTransition* t = &transitionTable[i];
        // 判断当前状态和条件是否匹配
        if (t->current_status == key->status && t->condition == current_cond) {
            // 如果存在动作，则执行动作
            if(t->action != NULL)
            {
                t->action(key);
            }
            
            // 更新状态
            key->status = t->next_status;
            break;
        }
    }
}



/**
 * @brief 注册一个新按键
 * @param port GPIO端口
 * @param pin GPIO引脚号
 * @param press_callback 按下回调函数，可为NULL
 * @param release_callback 释放回调函数，可为NULL
 * @return 按键索引，失败返回-1
 */
int8_t KEY_Register(GPIO_TypeDef* port, uint16_t pin, void (*press_callback)(void), void (*release_callback)(void))
{
    // 检查是否超出最大按键数
    if (key_count >= MAX_KEYS) {
        return -1;
    }
    
    // 注册新按键
    Keys[key_count].port = port;
    Keys[key_count].pin = pin;
    Keys[key_count].pin_state = 0;
    Keys[key_count].status = KS_RELEASE;
    Keys[key_count].isPressed = 0;
    Keys[key_count].press_callback = press_callback;
    Keys[key_count].release_callback = release_callback;
    
    return key_count++;
}

/**
 * @brief 按键扫描函数，在定时器中断中调用
 * @return 无
 */
void KEY_ScanInTick(void)
{
    for(uint8_t i = 0; i < key_count; i++) {
        Keys[i].pin_state = HAL_GPIO_ReadPin(Keys[i].port, Keys[i].pin);
        key_status_check(&Keys[i]);
    }
}

/**
 * @brief 获取按键状态
 * @param key_index 按键索引
 * @return 1表示按下，0表示释放
 */
uint8_t KEY_IsPressed(uint8_t key_index)
{
    if (key_index < key_count) {
        return Keys[key_index].isPressed;
    }
    return 0;
}

/**
 * @brief 定时器中断回调函数
 * @param htim 定时器句柄
 * @return 无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // 此处留给用户自定义使用哪个定时器作为扫描定时器
    // 例如：if(htim->Instance == TIM2) { KEY_ScanInTick(); }
    
    // 以下为示例代码，实际应用中应根据具体使用的定时器修改
    if(htim->Instance == TIM2) {
        KEY_ScanInTick();
    }
} 

