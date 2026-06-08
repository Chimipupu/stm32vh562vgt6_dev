/**
 * @file app_main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン (for STM32H562VGT6)
 * @version 0.1
 * @date 2026-06-07
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

// ST Lib
#include "app_freertos.h"

// My Src
#include "pcb_define.h"
#include "app_main.h"
#include "app_uart_cmd.h"

// --------------------------------------------------------------------------
// [FreeRTOS関連]

// アプリメインタスク
osThreadId_t AppMainTaskHandle;
const osThreadAttr_t AppMainTask_Attr = {
    .name = "AppMainTask",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 256 * 4
};
static void _AppMainTask(void *p_args);

// UARTデバッグコマンドタスク
osThreadId_t DbgCmdTaskHandle;
const osThreadAttr_t DbgCmdTask_Attr = {
    .name = "DbgCmdTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 256 * 4
};
static void _DbgCmdTask(void *p_args);

// --------------------------------------------------------------------------
// [バックアップSRAM @2KB]
// NOTE: STM32H562VGT6にはバックアップSRAMの「BKPSRAM」が 4KB ある

// Bootカウント
uint32_t g_bkram_boot_cnt __attribute__((section(".bk_sram"))) = 0;

// --------------------------------------------------------------------------
// static void _rtc_update(void);

// --------------------------------------------------------------------------
// [Static]
#if 0
static void _rtc_update(void)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    static uint8_t s_prev_seconds;

    HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
    if(s_prev_seconds != stimestructureget.Seconds) {
        s_prev_seconds  = stimestructureget.Seconds;
#ifdef PRINT_RTC_UPDATE
        DBG_LPUART_PRINTF("RTC: 20%02d/%02d/%02d %02d:%02d:%02d\r\n",
                sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date, \
                stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
#endif // PRINT_RTC_UPDATE
    }
}
#endif

static void _AppMainTask(void *p_args)
{
    // バックアップSRAMのBootカウントをインクリメント
    DBG_LPUART_PRINTF("[DEBUG] Boot Cnt = %d\r\n", g_bkram_boot_cnt);
    g_bkram_boot_cnt++;

    DBG_LPUART_PRINTF("[AppMainTask]: Init\r\n");

    while (1)
    {
        HAL_GPIO_TogglePin(PCB_LED_PORT, PCB_LED_PIN);
        // _rtc_update(); // RTC更新
        osDelay(10);
    }
}

static void _DbgCmdTask(void *p_args)
{
    app_uart_cmd_init(NULL);

    DBG_LPUART_PRINTF("[DbgCmdTask]: Init\r\n");

    while (1)
    {
        app_uart_cmd_main();
        osDelay(50);
    }
}

// --------------------------------------------------------------------------
// [App]

/**
 * @brief LPUART経由でprintf()相当の出力
 */
void DBG_LPUART_PRINTF(const char *format, ...)
{
    char buffer[256];
    va_list args;
    int len;

    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    for (int i = 0; i < len && i < sizeof(buffer); i++)
    {
        while (!LL_LPUART_IsActiveFlag_TXE(LPUART1));
        LL_LPUART_TransmitData8(LPUART1, (uint8_t)buffer[i]);
    }
}

void app_main_init(void)
{
    DBG_LPUART_PRINTF("STM32H562VGT6 Develop by Chimipupu\r\n");

    // アプリのメイン用のFreeRTOSタスクを生成
    AppMainTaskHandle = osThreadNew(_AppMainTask, NULL, &AppMainTask_Attr);

    // UARTデバッグコマンドタスク用のFreeRTOSタスクを生成
    DbgCmdTaskHandle = osThreadNew(_DbgCmdTask, NULL, &DbgCmdTask_Attr);
}
// --------------------------------------------------------------------------