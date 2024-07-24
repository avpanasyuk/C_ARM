#include "timer.hpp"

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
extern uint32_t SystemClockHz; // usually determined in setup.cpp  as HAL_RCC_GetSysClockFreq();

avp::tTimerChain avp::TimerChain;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    avp::TimerLink *p = (avp::TimerLink *)avp::TimerChain.FindFirst(htim);
    if(p != nullptr) p->fCallBack();
} // HAL_TIM_PeriodElapsedCallback
