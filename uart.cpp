/**
  @file
  @author Alexander Panasyuk
  */

#include "uart.h"

using namespace avp;

UART_for_Port_Types::tLookupTableEntry UART_for_Port_Types::LookupTable[];
uint8_t UART_for_Port_Types::NumTableEntries = 0;

/// @brief called by interrupt handler when DMA transfer is completed and HAL_DMA_STATE_READY is set
///
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  const UART_for_Port_Types::pCallBacks_ *p =
    UART_for_Port_Types::GetCallBacks(huart);
  if(p != nullptr) p->pStartNewTX();
} // HAL_UART_TxCpltCallback

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  const UART_for_Port_Types::pCallBacks_ *p =
    UART_for_Port_Types::GetCallBacks(huart);
  if(p != nullptr) p->pErrorCallback(HAL_UART_GetError(huart));
} // HAL_UART_ErrorCallback

//  we are receiving data byte by byte using interrupt, and storing them into RX_buffer
// from where they can be read in foreground
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  const UART_for_Port_Types::pCallBacks_ *p =
    UART_for_Port_Types::GetCallBacks(huart);
  if(p != nullptr) p->pRX_Callback();
} // HAL_UART_RxCpltCallback



