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
#define LPUART_RX_BUF_SIZE    128
static volatile uint8_t s_lpuart_rx_buf[LPUART_RX_BUF_SIZE];
#define UART_CMD_RX_BUF_SIZE    64
static volatile uint8_t s_uart_cmd_rx_buf[UART_CMD_RX_BUF_SIZE];
static uint8_t s_rx_buf_idx = 0;
static bool s_rx_uart_cmd_flg = false;
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
        osDelay(20);
    }
}

static void _DbgCmdTask(void *p_args)
{
    bool ret;

    DBG_LPUART_PRINTF("[DbgCmdTask]: Init\r\n");

    while (1)
    {
        if(s_rx_uart_cmd_flg != false) {
            ret = dbg_cmd_ready((uint8_t *) &s_uart_cmd_rx_buf[0]);
            if (ret != false) {
                DBG_LPUART_PRINTF("[DEBUG] Cmd: %s\r\n", s_uart_cmd_rx_buf);

                // TODO: コマンドのテーブル検索とコールバック関数の実行の実装

                // バッファ初期化
                memset((void *) &s_uart_cmd_rx_buf[0], 0x00, sizeof(s_uart_cmd_rx_buf));
            }
        }

        osDelay(100);
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
    uint8_t i;
    uint8_t *p_ptr = p_cmd_buf;

    for(i = 0; i < s_rx_buf_idx; i++)
    {
        if((s_lpuart_rx_buf[i] == '\r') || (s_lpuart_rx_buf[i] == '\n')) {
            break;
        }

        *p_ptr = s_lpuart_rx_buf[i];
        p_ptr++;
    }

    // 変数初期化
    memset((void *)&s_lpuart_rx_buf[0], 0x00, LPUART_RX_BUF_SIZE);
    s_rx_buf_idx = 0;
    s_rx_uart_cmd_flg = false;

    return true;
}

void lpuart1_irq_handler(void)
{
    uint8_t tmp;

#if 0
    // ORE （オーバーランエラー）
    if (LL_LPUART_IsActiveFlag_ORE(LPUART1)) {
        LL_LPUART_ClearFlag_ORE(LPUART1);
    }
#endif

    /**
    * @brief RXFNE (Receive FIFO Not Empty) をチェック
    * @note LPUARTのFIFOサイズ = 8Byte
    * @note RXFNEの割り込みはFIFOが1/8埋まったら来る
    * @note CPUが割り込みハンドラに来るまでに残りの7Byteは埋まるからORE（オーバーランエラー）は起きにくい、はず
    */
    while (LL_LPUART_IsActiveFlag_RXNE_RXFNE(LPUART1))
    {
        tmp = (uint8_t)LL_LPUART_ReceiveData8(LPUART1);
        if ((tmp == '\r') || (tmp == '\n')) {
            s_rx_uart_cmd_flg = true;
        }

        s_lpuart_rx_buf[s_rx_buf_idx] = tmp;
        s_rx_buf_idx = (s_rx_buf_idx + 1) % LPUART_RX_BUF_SIZE;
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