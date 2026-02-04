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

void app_main_init(void)
{
    s_usb_cdc_buf_data_len = sprintf(( char *)s_usb_cdc_tx_buf,"STM32H562VGT6 Develop by Chimipupu\r\n");

    // ADC開始
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)&s_adc_val,1);
}

void app_main(void)
{
    static uint8_t s_prev_seconds ;
    s_tick = HAL_GetTick();
    s_tick_usb_device = s_tick;
    s_tick_usb_cdc_tx = s_tick;
    s_tick_button     = s_tick;

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
        } else {
            s_tick_button = s_tick + 500;
            RTC_DateTypeDef sdatestructureget;
            RTC_TimeTypeDef stimestructureget;
            int text_lenth;

            /* Get the RTC current Time */
            HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
            /* Get the RTC current Date */
            HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);

            if(s_prev_seconds  != stimestructureget.Seconds) {
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