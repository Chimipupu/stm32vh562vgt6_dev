/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2026-06-04
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

// ST Lib
#include "app_freertos.h"

// My Src
#include "app_main.h"
#include "app_util.h"

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
static void _rtc_update(void);

#ifdef DBG_APP
static const uint32_t g_ref_val = 0x12345678;
static const uint32_t g_rev_exp_val = 0x78563412;
static const uint32_t g_rev16_exp_val = 0x34127856;
static bool _mcu_test(void);
#endif // DBG_APP
// --------------------------------------------------------------------------
// [Static]

static void _AppMainTask(void *p_args)
{
    printf("[AppMainTask]: Init\r\n");

    while (1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
        osDelay(100);
    }
}

static void _DbgCmdTask(void *p_args)
{
    printf("[DbgCmdTask]: Init\r\n");

    while (1)
    {
        _rtc_update(); // RTC更新
        osDelay(500);
    }
}

static void _rtc_update(void)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    static uint8_t s_prev_seconds;

    HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
    if(s_prev_seconds != stimestructureget.Seconds) {
        s_prev_seconds  = stimestructureget.Seconds;
        printf("RTC: 20%02d/%02d/%02d %02d:%02d:%02d\r\n",
                sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date, \
                stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
    }
}

#ifdef DBG_APP
static bool _mcu_test(void)
{
    DWT_Init();

    volatile uint32_t start_cycles, end_cycles, total_cycles;
    volatile uint32_t rev_val, rev16_val;

    printf("Start MCU Test\r\n");

    // CPUサイクル取得 @開始
    start_cycles = DWT_GetCPUCycleCount();

    // [H/W(CPU)でのエンディアン変換テスト]
    rev_val   = HW_Endian_32bit(g_ref_val); // 期待値: 0x78563412
    rev16_val = HW_Endian_16bit(g_ref_val); // 期待値: 0x34127856

    printf("End MCU Test\r\n");

    // CPUサイクル取得 @終了
    end_cycles = DWT_GetCPUCycleCount();
    total_cycles = end_cycles - start_cycles;
    printf("Total CPU Cycle = %d\r\n", total_cycles);

    // テスト結果確認
    if((rev_val != g_rev_exp_val) || (rev16_val != g_rev16_exp_val)) {
        return false; // テストNG
    } else {
        return true; // テストOK
    }
}
#endif // DBG_APP

// --------------------------------------------------------------------------
// [App]

void app_main_init(void)
{
    printf("STM32H562VGT6 Develop by Chimipupu\r\n");

#ifdef DBG_APP
    _mcu_test();
#endif // DBG_APP

    // アプリのメイン用のFreeRTOSタスクを生成
    AppMainTaskHandle = osThreadNew(_AppMainTask, NULL, &AppMainTask_Attr);

    // UARTデバッグコマンドタスク用のFreeRTOSタスクを生成
    DbgCmdTaskHandle = osThreadNew(_DbgCmdTask, NULL, &DbgCmdTask_Attr);
}
// --------------------------------------------------------------------------