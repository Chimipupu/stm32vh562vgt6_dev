/**
 * @file app_uart_cmd.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリ: UARTコマンド
 * @version 0.1
 * @date 2026-06-07
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

// My Src
#include "app_uart_cmd.h"
#include "pcb_define.h"
#include "app_main.h"
#include "app_util.h"
#include <string.h>

// --------------------------------------------------------------------------
#define LPUART_RX_BUF_SIZE    128
static volatile uint8_t s_lpuart_rx_buf[LPUART_RX_BUF_SIZE];
static uint8_t s_rx_buf_idx = 0;

#define UART_CMD_RX_BUF_SIZE    64
static volatile uint8_t s_uart_cmd_rx_buf[UART_CMD_RX_BUF_SIZE];
static bool s_rx_uart_cmd_flg = false;

E_APP_UART_CMD_RESULT _cmd_help(void *p_args);
// 基本コマンド
static const app_uart_cmd_tbl_t s_basic_cmd_tbl[] = {
    {"help", "?", _cmd_help},
};
static const uint8_t BASIC_CMD_NUM = sizeof(s_basic_cmd_tbl) / sizeof(s_basic_cmd_tbl[0]);

static app_uart_cmd_tbl_t *ps_cmd_tbl = NULL;
static E_APP_UART_CMD_TYPE s_cmd_type = CMD_TYPE_BASIC;
static uint8_t s_cmd_num;
static uint8_t s_rx_cmd_no;
static app_uart_cmd_config_t s_cmd_config;
bool _cmd_ready(uint8_t *p_cmd_buf);
// --------------------------------------------------------------------------
// [Static]

bool _cmd_ready(uint8_t *p_cmd_buf)
{
    bool ret = false;

    uint8_t i;
    uint8_t *p_ptr = p_cmd_buf;
    app_uart_cmd_tbl_t *p_tbl;

    for(i = 0; i < s_rx_buf_idx; i++)
    {
        if((s_lpuart_rx_buf[i] == '\r') || (s_lpuart_rx_buf[i] == '\n')) {
            break;
        }

        *p_ptr = s_lpuart_rx_buf[i];
        p_ptr++;
    }

    p_tbl = ps_cmd_tbl;

    // テーブル検索
    for(i = 0; i < s_cmd_num; i++)
    {
        // バッファとテーブルのコマンド文字列を比較
        if(strcmp(p_tbl->p_cmd_str, (const char *) p_cmd_buf) == 0) {
            DBG_LPUART_PRINTF("[DEBUG] Cmd RX: %s\r\n", p_cmd_buf);
            s_rx_cmd_no = i;
            ret = true;
            break;
        }
        p_tbl++;
    }

    // 知らんコマンドなので「?」を返す
    if(ret == false) {
        DBG_LPUART_PRINTF("?\r\n");
    }

    // 変数初期化
    memset((void *)&s_lpuart_rx_buf[0], 0x00, LPUART_RX_BUF_SIZE);
    memset((void *) &s_uart_cmd_rx_buf[0], 0x00, UART_CMD_RX_BUF_SIZE);
    s_rx_buf_idx = 0;
    s_rx_uart_cmd_flg = false;

    return ret;
}

E_APP_UART_CMD_RESULT _cmd_help(void *p_args)
{
    uint8_t i;

    DBG_LPUART_PRINTF("Help CMD Exec\r\n");

    // 基本コマンドを表示
    DBG_LPUART_PRINTF("Basic CMD List: NO, Cmd, Short Cmd\r\n");
    for(i = 0; i < BASIC_CMD_NUM; i++)
    {
        DBG_LPUART_PRINTF("%d, %s, %s\r\n", i, s_basic_cmd_tbl[i].p_cmd_str, s_basic_cmd_tbl[i].p_cmd_str_short);
    }

    // 拡張コマンドを表示
    if(s_cmd_config.p_ext_cmd_tbl != NULL) {
        DBG_LPUART_PRINTF("Ext CMD List: NO, Cmd, Short Cmd\r\n");
        for(i = 0; i < s_cmd_config.ext_cmd_num; i++)
        {
            DBG_LPUART_PRINTF("%d, %s, %s\r\n", i, s_cmd_config.p_ext_cmd_tbl[i].p_cmd_str, s_cmd_config.p_ext_cmd_tbl[i].p_cmd_str_short);
        }
    }

    return CMD_RESULT_EXEC_OK;
}

// --------------------------------------------------------------------------
// [APP]

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

void app_uart_cmd_init(app_uart_cmd_config_t *p_cmd_config)
{
    s_cmd_num = BASIC_CMD_NUM;
    s_cmd_type = CMD_TYPE_BASIC;
    ps_cmd_tbl = (app_uart_cmd_tbl_t *) &s_basic_cmd_tbl[0];

    // フェールセーフ #1: 引数のヌルポをチェック)
    if(p_cmd_config == NULL) {
        return;
    }

    // フェールセーフ #2: 引数のコマンドテーブルのチェック
    if((p_cmd_config->p_ext_cmd_tbl == NULL) || (p_cmd_config->ext_cmd_num == 0)) {
        return;
    }

    s_cmd_config = *p_cmd_config;
    s_cmd_type = CMD_TYPE_EXT;
    s_cmd_num = s_cmd_config.ext_cmd_num;
    ps_cmd_tbl = p_cmd_config->p_ext_cmd_tbl;
}

void app_uart_cmd_main(void)
{
    bool ret;

    if(s_rx_uart_cmd_flg != false) {
        ret = _cmd_ready((uint8_t *) &s_uart_cmd_rx_buf[0]);
        if (ret != false) {
            // コマンド実行
            if(ps_cmd_tbl[s_rx_cmd_no].pfunc != NULL) {
                ps_cmd_tbl[s_rx_cmd_no].pfunc(NULL);
            }
        }
    }
}
// --------------------------------------------------------------------------