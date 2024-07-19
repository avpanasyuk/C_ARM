#pragma once

#include <stm32f3xx_hal.h>
// #include "Error.h"
#include "../C_General/Chain.h"
#include "../C_General/MyMath.h"

extern uint32_t SystemClockHz; // usually determined in setup.cpp  as HAL_RCC_GetSysClockFreq();

namespace avp {
  extern Chain TimerChain;

  struct TimerLink: public Chain::Link {
    TimerLink(TIM_HandleTypeDef *htim_, void (*fCallBack_)()):
      Chain::Link(&TimerChain), htim(htim_), fCallBack(fCallBack_) {
    }
    TIM_HandleTypeDef *htim;
    void (*fCallBack)();
    class Comparator: public Chain::Link::Comparator {
      TIM_HandleTypeDef *htim;
      public:
      Comparator(TIM_HandleTypeDef *htim_): htim(htim_) {}
      virtual bool IsIt(const Chain::Link *p) const {return ((const TimerLink *)p)->htim == htim; };
    }; // class Comparator

  };

  template<TIM_HandleTypeDef *htim, void (*fCallBack)()>
  struct Timer {
    static TimerLink Link;

    static void SetInterval(uint64_t Interval_us) {
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




