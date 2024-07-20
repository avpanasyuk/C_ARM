#ifndef UART1_HPP_INCLUDED
#include <stdint.h>
// #include "C_General/UART.h"
namespace avp {
  static const inline pCallBacks_ *GetCallBacks(const UART_HandleTypeDef *puart) {
    for(uint8_t eI=0; eI < NumTableEntries; ++eI)
      if(LookupTable[eI].puart == puart) return LookupTable[eI].pCallBacks;
    return nullptr;
  } // GetCallBacks
protected:
  static struct tLookupTableEntry {
    const UART_HandleTypeDef *puart;
    const pCallBacks_ *pCallBacks;
  } LookupTable[5]; // To fit all UART ports
  static uint8_t NumTableEntries;
      LookupTable[NumTableEntries++] = {puart, &pCallBacks};
}; // namespace avp