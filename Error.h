#pragma once

#include <stm32f3xx_hal_def.h>
#include "../C_General/Error.h"

#define AVP_HAL_ASSERT(h_ptr,exp) do{ HAL_StatusTypeDef status = (exp); if(status != HAL_OK) \
    AVP_ERROR("Expression \"" #exp "\" failed with status %u, in state %u!\n", status, (h_ptr)->State); }while(0)

#define AVP_HAL_ASSERT_WITH_CODE(h_ptr,exp) do{ HAL_StatusTypeDef status = (exp); if(status != HAL_OK) \
    AVP_ERROR("Expression \"" #exp "\" failed with code %lu, status %u, in state %u!\n",(h_ptr)->ErrorCode, status, (h_ptr)->State); }while(0)
