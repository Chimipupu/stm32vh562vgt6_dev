/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン (for STM32H562VGT6)
 * @version 0.1
 * @date 2026-06-07
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
#include "cmsis_os2.h"
#include "cordic.h"
#include "crc.h"
#include "i2c.h"
#include "icache.h"
#include "usart.h"
#include "rng.h"
#include "rtc.h"
#include "spi.h"
#include "gpio.h"

// MyApp Lib

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
// #define PRINT_RTC_UPDATE

// --------------------------------------------------------------------------
// [Define]

// --------------------------------------------------------------------------
// [FreeRTOS関連]
extern osThreadId_t AppMainTaskHandle;
extern const osThreadAttr_t AppMainTask_Attr;
// --------------------------------------------------------------------------
void DBG_LPUART_PRINTF(const char *format, ...);
bool dbg_cmd_ready(uint8_t *p_cmd_buf);
void lpuart1_irq_handler(void);
void app_main_init(void);
// --------------------------------------------------------------------------

#endif // APP_MAIN_H