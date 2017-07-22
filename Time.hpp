#ifndef TIME_HPP_INCLUDED
#define TIME_HPP_INCLUDED

/**
  @file
  @author Alexander Panasyuk
  */

#include <AVP_LIBS/General/Time.h>

namespace avp {
  namespace arm {
    template<void (*Func)(), uint32_t Period>
    class RunPeriodically: public avp::RunPeriodically<&HAL_GetTick, Func, Period> {};
  } // namespace arm
} // namespace avp


#endif /* TIME_HPP_INCLUDED */
