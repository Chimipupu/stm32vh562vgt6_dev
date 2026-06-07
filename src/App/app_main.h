/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2026-06-04
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef APP_MAIN_H
#define APP_MAIN_H

// C Std Lib
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

// ST Lib
#include "main.h"
#include "gpdma.h"
#include "icache.h"
#include "rtc.h"
#include "gpio.h"

// MyApp Lib

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#define DBG_APP

// --------------------------------------------------------------------------
#ifndef KEY_Pin
#define KEY_Pin GPIO_PIN_13
#endif
#ifndef KEY_GPIO_Port
#define KEY_GPIO_Port GPIOC
#endif
#ifndef LED_Pin
#define LED_Pin GPIO_PIN_2
#endif
#ifndef LED_GPIO_Port
#define LED_GPIO_Port GPIOB
#endif

// --------------------------------------------------------------------------
void app_main_init(void);
void app_main(void);

// --------------------------------------------------------------------------

#endif // APP_MAIN_H