#pragma once

#include <stm32f3xx_hal.h>
// #include "Error.h"
#include "../C_General/Chain.h"
#include "../C_General/MyMath.h"

extern uint32_t SystemClockHz; // usually determined in setup.cpp  as HAL_RCC_GetSysClockFreq();

namespace avp {
  extern ChainByPointer TimerChain;

/**
this is a service class which allows tro link all the "Timer"s in a chain, so interrupt handler can
go through it and find callbacks.
*/
  struct TimerLink: public ChainByPointer::Link {
    TimerLink(TIM_HandleTypeDef *htim_, void (*fCallBack_)()):
      ChainByPointer::Link(htim_, &TimerChain), fCallBack(fCallBack_) {
    }
    void (*fCallBack)();
  };

/**
This is purely static class, no instances are required
*/
  template<TIM_HandleTypeDef *htim, void (*fCallBack)()>
  struct Timer {
    static TimerLink Link;

    static void SetInterval_us(uint64_t Interval_us) {
      uint32_t Clocks = Interval_us*SystemClockHz/1000000UL;
      uint16_t Sqrt = avp::IntSqrt<uint16_t, uint32_t>(Clocks);

      htim->Init.Prescaler = Sqrt - 1;
      htim->Init.Period = Clocks/Sqrt - 1;
      AVP_ASSERT(HAL_TIM_Base_Init(htim) == HAL_OK);
    } // SetInterval

    static void Start() {
     AVP_ASSERT(HAL_TIM_Base_Start(htim) == HAL_OK);
    } // Start

    static void Stop() {
     AVP_ASSERT(HAL_TIM_Base_Stop(htim) == HAL_OK);
    } // Start
  }; // class Timer

  template<TIM_HandleTypeDef *htim, void (*fCallBack)()>
  TimerLink Timer<htim,fCallBack>::Link = {htim, fCallBack};
} // namespace avp




