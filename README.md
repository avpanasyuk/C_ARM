# C_ARM

ARM / STM32 (Cortex-M) C++ peripheral and helper library — the ARM-side sibling of
[`C_General`](https://github.com/avpanasyuk/C_General), `C_ESP`, and `C_AVR`,
vendored as a git submodule by the author's STM32 firmware projects.

## Contents

| File | Provides |
|------|----------|
| `STM32F.hpp` | STM32F-family register / peripheral definitions and helpers. |
| `gpio.hpp` | GPIO abstraction. |
| `uart.h` / `uart.cpp` | UART driver. |
| `timer.hpp` / `timer.cpp` | Timer helpers. |
| `USB_CDC_IO.hpp` / `.cpp` | USB CDC (virtual COM port) I/O. |
| `simulated_eeprom.h` / `.cpp` | EEPROM emulation in flash. |
| `MAX30102.hpp` | Driver for the MAX30102 pulse-oximeter / heart-rate sensor. |
| `Error.h` | Error / debug hooks (mirrors `C_General`'s `debug_*`). |

Trunk: `development`. Consumed as a submodule alongside `C_General`.
