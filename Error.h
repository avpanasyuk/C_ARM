#pragma once

/// @cond
#include "MCU_HAL.h"
/// @endcond
#include "../C_General/Error.h"

#define AVP_HAL_ASSERT(h_ptr,exp) do{ HAL_StatusTypeDef status = (exp); if(status != HAL_OK) \
    AVP_ERROR("Expression \"" #exp "\" failed with status %u, in state %u!\n", status, (h_ptr)->State); }while(0)

#define AVP_HAL_ASSERT_WITH_CODE(h_ptr,exp) do{ HAL_StatusTypeDef status = (exp); if(status != HAL_OK) \
    AVP_ERROR("Expression \"" #exp "\" failed with code %lu, status %u, in state %u!\n",(h_ptr)->ErrorCode, status, (h_ptr)->State); }while(0)

#define PRINTF_IF_HAL_ERROR(h, exp, printf_fun) \
HAL_StatusTypeDef s = (exp);\
if(s != HAL_OK) {\
printf_fun("HAL error in %s: %hu, code: %lu on line %d\n", \
            __PRETTY_FUNCTION__, s, h.ErrorCode, __LINE__);


/**
 * returns HAL_StatusTypeDef
 */
#define IF_HAL_ERROR_PRINTF_RETURN(h, exp, printf_fun) do {\
HAL_StatusTypeDef s = (exp);\
if(s != HAL_OK) {\
  printf_fun("HAL error in %s: %hu, code: %lu on line %d\n", \
            __PRETTY_FUNCTION__, s, h.ErrorCode, __LINE__); \
            return s; }}while(0)

/**
 * returns HAL_StatusTypeDef
 */
#define IF_HAL_ERROR_DEBUG_RETURN(h, exp) IF_HAL_ERROR_PRINTF_RETURN(h,exp, debug_printf)



