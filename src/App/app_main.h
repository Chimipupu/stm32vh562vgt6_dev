/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2026-02-04
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

// C Std Lib
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// ST Lib

// BSP
#include "main.h"
#include "adc.h"
#include "gpdma.h"
#include "icache.h"
#include "rtc.h"
#include "usb.h"
#include "app_usbx_device.h"
#include "gpio.h"
#include "board.h"

// MyApp Lib

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#define DBG_APP

// --------------------------------------------------------------------------
void app_main_init(void);
void app_main(void);

#endif // APP_MAIN_H