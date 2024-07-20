#include "timer.hpp"

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
extern uint32_t SystemClockHz; // usually determined in setup.cpp  as HAL_RCC_GetSysClockFreq();

avp::Chain avp::TimerChain;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    avp::TimerLink *p = (avp::TimerLink *)avp::TimerChain.FindFirst(avp::TimerLink::Comparator(htim));
    if(p != nullptr) p->fCallBack();
} // HAL_TIM_PeriodElapsedCallback