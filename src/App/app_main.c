/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2026-06-04
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"
#include "app_util.h"

// --------------------------------------------------------------------------
static uint8_t s_printf_data_buf[256];
static uint32_t s_printf_data_length;
static uint32_t s_tick_button;
static uint32_t s_tick;

#ifdef DBG_APP
static const uint32_t g_ref_val = 0x12345678;
static const uint32_t g_rev_exp_val = 0x78563412;
static const uint32_t g_rev16_exp_val = 0x34127856;
static bool _mcu_test(void);
#endif // DBG_APP

// --------------------------------------------------------------------------
// [Static]

#ifdef DBG_APP
static bool _mcu_test(void)
{
    DWT_Init();

    // volatile uint32_t start_cycles, end_cycles, total_cycles;
    volatile uint32_t rev_val, rev16_val;

    // CPUサイクル取得 @開始
    // start_cycles = DWT_GetCPUCycleCount();

    // [H/W(CPU)でのエンディアン変換テスト]
    rev_val   = HW_Endian_32bit(g_ref_val); // 期待値: 0x78563412
    rev16_val = HW_Endian_16bit(g_ref_val); // 期待値: 0x34127856

    // CPUサイクル取得 @終了
    // end_cycles = DWT_GetCPUCycleCount();
    // total_cycles = end_cycles - start_cycles;

    // テスト結果確認
    if((rev_val != g_rev_exp_val) || (rev16_val != g_rev16_exp_val)) {
        return false; // テストNG
    } else {
        return true; // テストOK
    }
}
#endif // DBG_APP

void app_main_init(void)
{
    memset(s_printf_data_buf, 0, sizeof(s_printf_data_buf));
    s_tick = HAL_GetTick();

    s_printf_data_length = sprintf(( char *)s_printf_data_buf,"STM32H562VGT6 Develop by Chimipupu\r\n");

#ifdef DBG_APP
    _mcu_test();
#endif // DBG_APP
}

void app_main(void)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    static uint8_t s_prev_seconds ;

    s_tick = HAL_GetTick();

    // 基板のボタンをポーリング(100ms周期)
    if(s_tick >= s_tick_button) {
        if(board_button_getstate()) {
            s_tick_button = s_tick + 100;
            board_led_toggle();
            s_printf_data_length = sprintf((char *)s_printf_data_buf, "Key Pressed\r\n");
        }
        // 500ms毎にRTCを更新
        else {
            s_tick_button = s_tick + 500;

            /* Get the RTC current Time */
            HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
            /* Get the RTC current Date */
            HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);

            // 1秒おきにRTCの時刻を表示
            if(s_prev_seconds != stimestructureget.Seconds) {
                s_prev_seconds  = stimestructureget.Seconds;
                board_led_set(1);
                s_printf_data_length = sprintf((char *) &s_printf_data_buf,
                                                "20%02d.%02d.%02d %02d:%02d:%02d\r\n",
                                                sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date, \
                                                stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
            } else {
                board_led_set(0);
            }
        }
    }
}