# STM32G0B1CBT6 評価F/W開発

## 開発環境

- マイコン: `STM32G0B1CBT6`
  - CPU: ARM Cortex-M33
  - FPU: 単精度FPU
  - Clock: 250MHz
  - Flash: 2MB
  - SRAM: 640KB
- OS
  - FreeRTOS (CMSIS RTOS 2)
- コンパイラ: Clang (`st-arm-clang 19.1.6`) 
  - 最適化: debug
- ツールチェイン
  - CMake
  - STM32CubeMX
  - STM32CubeIDE (VSCode版)
- デバッグ
  - デバッガ: `ST-LINK/V2-1`
    - デバッグI/F: SWD
  - printf()デバッグ
    - LPUART
      - TX: PA9ピン
      - RX: PA10ピン
      - 115200bps 8N1

## メモリ使用量

```shell
[build] Memory region         Used Size  Region Size  %age Used
[build]              RAM:       13872 B       640 KB      2.12%
[build]            FLASH:       35120 B         2 MB      1.67%
```

## ピンアサイン

![alt text](doc/STM32H562VGT6_CubeMXピン設定_20260607.png)

