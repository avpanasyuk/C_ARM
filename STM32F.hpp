#ifndef STM32F_HPP_INCLUDED#define STM32F_HPP_INCLUDED/**@file@author Alexander Panasyuk*//// @cond
#include <stdint.h>#include <stm32f303xe.h>#include <core_cm4.h>#include <cmsis_gcc.h>/// @endcond
namespace avp {  class InterruptStopper {      uint32_t prim;    public:      InterruptStopper() {        prim = __get_PRIMASK();        __disable_irq();      }      ~InterruptStopper() {        if(!prim) __enable_irq();      }  }; // class InterruptStopper} // namespace avp#endif /* STM32F_HPP_INCLUDED */