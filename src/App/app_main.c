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
#include "app_main.h"
#include "app_util.h"

extern UX_SLAVE_CLASS_CDC_ACM  *cdc_acm;

static uint16_t s_adc_val;
static uint8_t s_usb_cdc_tx_buf[64];
static uint32_t s_usb_cdc_buf_data_len;
static unsigned long s_actual_length;
static uint32_t s_step = UX_STATE_RESET;
static uint32_t ux_status = UX_STATE_RESET;
static uint32_t s_tick;
static uint32_t s_tick_usb_device;
static uint32_t s_tick_usb_cdc_tx;
static uint32_t s_tick_button;

#ifdef DBG_APP
volatile const uint32_t g_ref_val = 0x12345678;
volatile const uint32_t g_rev_exp_val = 0x78563412;
volatile const uint32_t g_rev16_exp_val = 0x34127856;
static void dbg_mcu_test(void);

static void dbg_mcu_test(void)
{
    DWT_Init();

    volatile uint32_t start_cycles, end_cycles, total_cycles;
    volatile uint32_t rev_val, rev16_val;

    // CPUサイクル取得 @開始
    start_cycles = DWT_GetCPUCycleCount();

    // [H/W(CPU)でのエンディアン変換テスト]
    rev_val   = HW_Endian_32bit(g_ref_val); // 期待値: 0x78563412
    rev16_val = HW_Endian_16bit(g_ref_val); // 期待値: 0x34127856

    // CPUサイクル取得 @終了
    end_cycles = DWT_GetCPUCycleCount();
    total_cycles = end_cycles - start_cycles;

    // テスト結果確認
    if((rev_val != g_rev_exp_val) || (rev16_val != g_rev16_exp_val)) {
        __BKPT(0); // テストNG
    } else {
        __BKPT(1); // テストOK
    }
}
#endif // DBG_APP

void app_main_init(void)
{
    s_tick = HAL_GetTick();
    s_tick_usb_device = s_tick;
    s_tick_usb_cdc_tx = s_tick;
    s_tick_button     = s_tick;

    s_usb_cdc_buf_data_len = sprintf(( char *)s_usb_cdc_tx_buf,"STM32H562VGT6 Develop by Chimipupu\r\n");

    // ADC開始
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)&s_adc_val,1);

#ifdef DBG_APP
    dbg_mcu_test();
#endif // DBG_APP
}

void app_main(void)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    static uint8_t s_prev_seconds ;

    s_tick = HAL_GetTick();

    // USBデバイス処理(1ms周期)
    if(s_tick >= s_tick_usb_device) {
        s_tick_usb_device = s_tick + 1;
        ux_device_stack_tasks_run();
    }

    // ボタンポーリング処理(100ms周期)
    if(s_tick >= s_tick_button) {
        if(board_button_getstate()) {
            s_tick_button = s_tick + 100;
            board_led_toggle();
            s_usb_cdc_buf_data_len = sprintf(( char *)s_usb_cdc_tx_buf,"Key Pressed\r\n");
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
                s_usb_cdc_buf_data_len = sprintf((char *) &s_usb_cdc_tx_buf,
                                                "20%02d.%02d.%02d %02d:%02d:%02d ,%dmV\r\n",
                                                sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date, \
                                                stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds, \
                                                (((uint32_t)s_adc_val)*3300)>>10);
            } else {
                board_led_set(0);
            }
        }
    }

    // USB CDC送信処理(1ms or 1000ms周期)
    if(s_tick >= s_tick_usb_cdc_tx) {
        s_tick_usb_cdc_tx = s_tick + 1;
        if(cdc_acm != UX_NULL) {
            switch(s_step)
            {
                case UX_STATE_RESET:
                    ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, s_usb_cdc_tx_buf,s_usb_cdc_buf_data_len, &s_actual_length);
                    if (ux_status != UX_STATE_WAIT) {
                        s_step = UX_STATE_RESET;
                        break;
                    }
                    s_step = UX_STATE_WAIT;
                    break;

                case UX_STATE_WAIT:
                    /* Continue to run state machine.  */
                    ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UX_NULL, 0, &s_actual_length);
                    /* Check if there is  fatal error.  */
                    if (ux_status < UX_STATE_IDLE) {
                        /* Reset state.  */
                        s_step = UX_STATE_RESET;
                        break;
                    }
                    /* Check if dataset is transmitted */
                    if (ux_status <= UX_STATE_NEXT) {
                        s_step = UX_STATE_RESET;
                        s_tick_usb_cdc_tx = s_tick + 1000;
                    }
                    /* Keep waiting.  */
                    break;

                default:
                    break;
            }
        }
    }
}