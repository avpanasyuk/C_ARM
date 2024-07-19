#pragma once

#include <stm32f3xx_hal_def.h>
#include "../C_General/Error.h"

#define AVP_HAL_ASSERT(h_ptr,exp) do{ HAL_StatusTypeDef status = (exp); if(status != HAL_OK) \
    AVP_ERROR("Expression \"" #exp "\" failed with code %lu!\n",(h_ptr)->ErrorCode); }while(0)
