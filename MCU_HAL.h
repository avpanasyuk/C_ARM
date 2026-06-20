#ifndef MCU_HAL_H_INCLUDED
#define MCU_HAL_H_INCLUDED
/**
 * @file MCU_HAL.h
 * @brief Family-portable HAL umbrella for C_ARM. Picks the correct STM32 family HAL
 *   from the CubeMX device define so C_ARM builds across families (F1/F3/...). The
 *   umbrella pulls in hal_def + every enabled module header, so includers need only this.
 */
#if defined(STM32F1) || \
    defined(STM32F100xB) || defined(STM32F100xE) || \
    defined(STM32F101x6) || defined(STM32F101xB) || defined(STM32F101xE) || defined(STM32F101xG) || \
    defined(STM32F102x6) || defined(STM32F102xB) || \
    defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG) || \
    defined(STM32F105xC) || defined(STM32F107xC)
#  include <stm32f1xx_hal.h>
#elif defined(STM32F3) || \
      defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F302xC) || defined(STM32F302xE) || \
      defined(STM32F303x8) || defined(STM32F303xC) || defined(STM32F303xE) || \
      defined(STM32F373xC) || defined(STM32F378xx) || defined(STM32F334x8) || \
      defined(STM32F318xx) || defined(STM32F328xx) || defined(STM32F358xx) || defined(STM32F398xx)
#  include <stm32f3xx_hal.h>
#elif defined(STM32F4)
#  include <stm32f4xx_hal.h>
#else
#  error "C_ARM/MCU_HAL.h: unknown STM32 family - add its HAL umbrella include here"
#endif
#endif // MCU_HAL_H_INCLUDED
