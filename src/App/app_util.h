/**
 * @file app_util.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリユーティリティ
 * @version 0.1
 * @date 2026-02-07
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#ifndef APP_UTIL_H
#define APP_UTIL_H

/**
 * @brief CPUサイクルカウントの取得有効
 * @mote ARM Cortex-M33のDWTでCPUサイクルカウントで取得
 */
void DWT_Init(void)
{
    // TRCENAビットをセットして、DWTユニットへのアクセスを許可 (CoreDebug->DEMCR)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // DWTのロック解除
#ifdef DWT_LAR_KEY
    // NOTE: CMSISの定義にDWT_LARがあれば実施（通常0xC5ACCE55を書き込む）
    DWT->LAR = 0xC5ACCE55;
#endif

    // サイクルカウンタをリセット
    DWT->CYCCNT = 0;

    // サイクルカウンタを有効化 (DWT_CTRL)
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief CPUサイクルカウントの取得
 * @mote ARM Cortex-M33のDWTでCPUサイクルカウントを取得
 */
static inline uint32_t DWT_GetCPUCycleCount(void)
{
    return DWT->CYCCNT;
}

/**
 * @brief H/W(CPU)で32bitエンディアン変換 (REV命令)
 * @note 例: 0x12345678 -> 0x78563412
 */
static inline uint32_t HW_Endian_32bit(uint32_t value)
{
    return __REV(value);
}

/**
 * @brief H/W(CPU)で16bitエンディアン変換 (REV16命令)
 * @note 例: 0x12345678 -> 0x34127856
 */
static inline uint32_t HW_Endian_16bit(uint32_t value)
{
    return __REV16(value);
}

#endif // APP_UTIL_H