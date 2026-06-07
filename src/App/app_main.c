/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2026-06-07
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

// ST Lib
#include "app_freertos.h"

// My Src
#include "pcb_define.h"
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
// [バックアップSRAM @2KB]
// NOTE: STM32H562VGT6にはバックアップSRAMの「BKPSRAM」が 4KB ある

// Bootカウント
uint32_t g_bkram_boot_cnt __attribute__((section(".bk_sram"))) = 0;

// --------------------------------------------------------------------------
volatile static bool s_rx_uart_cmd_flg = false;

#define LPUART_RX_BUF_SIZE    256
static volatile uint8_t s_lpuart_rx_buf[LPUART_RX_BUF_SIZE];
static uint8_t s_rx_buf_idx = 0;

static void _rtc_update(void);

#ifdef DBG_APP

#endif // DBG_APP
// --------------------------------------------------------------------------
// [Static]

static void _rtc_update(void)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    static uint8_t s_prev_seconds;

    HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
    if(s_prev_seconds != stimestructureget.Seconds) {
        s_prev_seconds  = stimestructureget.Seconds;
        DBG_LPUART_PRINTF("RTC: 20%02d/%02d/%02d %02d:%02d:%02d\r\n",
                sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date, \
                stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
    }
}

#ifdef DBG_APP

#endif // DBG_APP

static void _AppMainTask(void *p_args)
{
    // バックアップSRAMのBootカウントをインクリメント
    DBG_LPUART_PRINTF("[DEBUG] Boot Cnt = %d\r\n", g_bkram_boot_cnt);
    g_bkram_boot_cnt++;

    DBG_LPUART_PRINTF("[AppMainTask]: Init\r\n");

    while (1)
    {
        HAL_GPIO_TogglePin(PCB_LED_PORT, PCB_LED_PIN);
        _rtc_update(); // RTC更新
        osDelay(100);
    }
}

static void _DbgCmdTask(void *p_args)
{
    bool ret;
    uint8_t cmd_buf[64] = {0};

    DBG_LPUART_PRINTF("[DbgCmdTask]: Init\r\n");

    while (1)
    {
        ret = dbg_cmd_ready(cmd_buf);
        if (ret != false) {
            DBG_LPUART_PRINTF("[DEBUG] Cmd: %s\r\n", cmd_buf);
            memset(&cmd_buf[0], 0x00, sizeof(cmd_buf));
        }

        osDelay(300);
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

bool dbg_cmd_ready(uint8_t *p_cmd_buf)
{
    bool ret = false;
    uint8_t i;
    uint8_t *p_ptr = p_cmd_buf;

    // NOTE: コマンドを渡す(将来的にはDMAにさせたい)
    if(s_rx_uart_cmd_flg != false) {
        for(i = 0; i < s_rx_buf_idx; i++)
        {
            *p_ptr = s_lpuart_rx_buf[i];
            p_ptr++;
        }

        ret = true;

        // 変数初期化
        memset((void *)&s_lpuart_rx_buf[0], 0x00, LPUART_RX_BUF_SIZE);
        s_rx_buf_idx = 0;
        s_rx_uart_cmd_flg = false;
    }

    return ret;
}

void lpuart1_irq_handler(void)
{
    volatile uint8_t tmp;

    /* RXNE */
    if (LL_LPUART_IsActiveFlag_RXNE(LPUART1)) {
        tmp = (uint8_t)LL_LPUART_ReceiveData8(LPUART1);

        // ASCIIの文字か特定の文字だけバッファで受け取る
        if ((tmp >= '0' && tmp <= '9') ||
            (tmp >= 'A' && tmp <= 'Z') || (tmp >= 'a' && tmp <= 'z') ||
            (tmp == '!') || (tmp == '?'))
        {
            s_lpuart_rx_buf[s_rx_buf_idx] = tmp;
            s_rx_buf_idx = (s_rx_buf_idx + 1) % LPUART_RX_BUF_SIZE;
        // コマンド受信 = デリミタ
        } else if ((s_rx_buf_idx > 0) && (tmp == '\r' || tmp == '\n')) {
            s_rx_uart_cmd_flg = true;
        }
    }
}

void app_main_init(void)
{
    DBG_LPUART_PRINTF("STM32H562VGT6 Develop by Chimipupu\r\n");

#ifdef DBG_APP
    _mcu_test();
#endif // DBG_APP

    // アプリのメイン用のFreeRTOSタスクを生成
    AppMainTaskHandle = osThreadNew(_AppMainTask, NULL, &AppMainTask_Attr);

    // UARTデバッグコマンドタスク用のFreeRTOSタスクを生成
    DbgCmdTaskHandle = osThreadNew(_DbgCmdTask, NULL, &DbgCmdTask_Attr);
}
// --------------------------------------------------------------------------