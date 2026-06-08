/**
 * @file app_uart_cmd.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリ: UARTコマンド
 * @version 0.1
 * @date 2026-06-07
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// --------------------------------------------------------------------------
typedef enum {
    CMD_TYPE_BASIC = 0x00, // 基本コマンド
    CMD_TYPE_EXT           // 拡張コマンド
} E_APP_UART_CMD_TYPE;

// コマンド実行結果
typedef enum {
    CMD_RESULT_NONE                 = 0x00, // コマンド処理なし
    CMD_RESULT_EXEC_OK              = 0x01, // コマンド実行成功
    CMD_RESULT_EXEC_ERROR           = 0x02, // コマンド実行失敗
    CMD_RESULT_EXEC_IN_PROGRESS     = 0x03, // コマンド実行中
    CMD_RESULT_ARGS_ERROR           = 0xF0, // コマンド引数エラー
    CMD_RESULT_UNKNOWN_ERROR        = 0xFF, // 不明なエラー
} E_APP_UART_CMD_RESULT;

// コマンドコールバック関数
typedef E_APP_UART_CMD_RESULT(*p_cmd_func)(void *p_args);

// コマンドテーブル構造体
typedef struct {
    const char *p_cmd_str;          // コマンド文字列
    const char *p_cmd_str_short;    // コマンド短縮文字列
    p_cmd_func pfunc;               // コールバック関数
} app_uart_cmd_tbl_t;

// 拡張コマンド設定構造体
typedef struct {
    app_uart_cmd_tbl_t *p_ext_cmd_tbl; // 拡張コマンドのテーブルポインタ
    uint8_t ext_cmd_num;               // 拡張コマンド数
} app_uart_cmd_config_t;

// --------------------------------------------------------------------------
void lpuart1_irq_handler(void);
void app_uart_cmd_init(app_uart_cmd_config_t *p_cmd_config);
void app_uart_cmd_main(void);
// --------------------------------------------------------------------------