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

// MyApp Lib

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#define DBG_APP
// #define DEBUG_PRINT_USB_CDC
// --------------------------------------------------------------------------

// BSP
#include "main.h"
#include "gpdma.h"
#include "icache.h"
#include "rtc.h"
#include "gpio.h"
#include "board.h"
#ifdef DEBUG_PRINT_USB_CDC
#include "usb.h"
#include "app_usbx_device.h"
#endif // DEBUG_PRINT_USB_CDC

// --------------------------------------------------------------------------
void app_main_init(void);
void app_main(void);

// --------------------------------------------------------------------------

#endif // APP_MAIN_H