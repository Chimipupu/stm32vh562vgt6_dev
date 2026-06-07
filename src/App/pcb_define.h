/**
 * @file pcb_define.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 基板定義 (STM32H562VGT6)
 * @version 0.1
 * @date 2026-06-04
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef PCB_DEFINE_H
#define PCB_DEFINE_H

#include "stm32h5xx_hal.h"

// -----------------------------------------------------------
// [コンパイルスイッチ]
#define PCB_WEACT_STM32H562VGT6

// -----------------------------------------------------------
#ifdef PCB_WEACT_STM32H562VGT6
// 基板青色LED @PB2
#define PCB_LED_PORT       GPIOB
#define PCB_LED_PIN        GPIO_PIN_2

// 基板ボタン @PC13
#define PCB_BUTTON_PORT    GPIOC
#define PCB_BUTTON_PIN     GPIO_PIN_13

#define UART_BAUD         921600
#endif // PCB_WEACT_STM32H562VGT6

// -----------------------------------------------------------

#endif // PCB_DEFINE_H