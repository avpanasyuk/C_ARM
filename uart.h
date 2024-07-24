#ifndef UART1_HPP_INCLUDED
#define UART1_HPP_INCLUDED

/**
  @file
  @author Alexander Panasyuk
  definition of class to be used as HW_IO_ in Port.h
  */
/// @cond
#include <stdint.h>
#include <string.h>
#include <stm32f3xx_hal.h>
#include <stm32f3xx_hal_dma.h>
#include <stm32f3xx_hal_uart.h>
// #include "CubeMX/Inc/usart.h"
#include "../C_General/Error.h"
#include "../C_General/IO.h"
#include "../C_General/Chain.h"
/// @endcond
// #include "C_General/UART.h"
// #include "err_codes.h"

namespace avp {
  typedef ChainByKey<UART_HandleTypeDef *> tUART_Chain;

  extern tUART_Chain UART_for_Port_Chain; //!< chain which allows to find callbacks from huart

  struct tUART_Link: public tUART_Chain::Link {
    void (*pStartNewTX)();
    void (*pErrorCallback)(uint32_t);
    void (*pRX_Callback)();

    tUART_Link(UART_HandleTypeDef *puart,
               void (*pStartNewTX_)(),
               void (*pErrorCallback_)(uint32_t),
               void (*pRX_Callback_)()):
      tUART_Chain::Link(puart, &UART_for_Port_Chain),
      pStartNewTX(pStartNewTX_), pErrorCallback(pErrorCallback_), pRX_Callback(pRX_Callback_) {}
  }; // struct UART_Link

#define T template<UART_HandleTypeDef *puart>

  T class UART_for_Port {
   protected:
    static tUART_Link UART_Link;
    static tStoreReceivedByte pStoreReceivedByte; ///< pointer to callback function which
    /// have to be specifies using Init function
    static tGetBlockToSend pGetBlockToSend; ///< pointer to callback function which
    /// have to be specifies using Init function
    static uint8_t NewByte;
    static bool TX_in_Progress; ///< mutex, does not allow to HAL_UART_Transmit_DMA twice
    // I do not want to rely on internal HAL_UART_Transmit_DMA mutex, because we are calling
    // pGetBlockToSend and if TX is busy what are we going to do with data
    static constexpr uint8_t ERR_STR_MAX_SZ = 254;
    static char ErrStr[ERR_STR_MAX_SZ+1];
    static bool InError;

    //! This function can be called from interrupt callback
    static void RX_Byte_IT() {
      if(!InError) {
        HAL_StatusTypeDef Status = HAL_UART_Receive_IT(puart, &NewByte,1);

        if(Status != HAL_OK) { //  && Status != HAL_BUSY) {
          snprintf(ErrStr,ERR_STR_MAX_SZ,"RX_Byte_IT: Status = %hu, Error = %lu\n",
                   Status, HAL_UART_GetError(puart));
          InError = true;
        }
      }
    } // RX_Byte_IT

   public:
    static void Init(tStoreReceivedByte pStoreReceivedByte_,  tGetBlockToSend pGetBlockToSend_) {
      pStoreReceivedByte = pStoreReceivedByte_;
      pGetBlockToSend = pGetBlockToSend_;
      if(UART_for_Port_Chain.FindFirst(puart) == nullptr) UART_for_Port_Chain.Append(&UART_Link); // FIXME - static constructor is not called for some reason
      RX_Byte_IT();
    } // Init


    /**
    * This is just a service function for TryToSend
    */

    static void Send(const uint8_t *p, size_t Sz) {
      TX_in_Progress = true;
      HAL_StatusTypeDef Status;
      if((Status = HAL_UART_Transmit_DMA(puart,(uint8_t *)p,Sz)) != HAL_OK) {
        snprintf(ErrStr,ERR_STR_MAX_SZ,"TryToSend: Status = %x, Error = %lu\n",
                 Status, HAL_UART_GetError(puart));
        InError = true;
        TX_in_Progress = false;
      }
    } // Send

    /**
    * this function checks whether there is anything to send and sends it using DMA
    * We can try to make it reentarable and call from TX_Done which itself is called from  the interrupt callback
    * HAL_UART_TxCpltCallback, this way TX will be faster. Or we can rely on calling it from the Port::cycle() or such.
    */
    static void TryToSend() __attribute__((section (".ccmram"))) { //
      // to go into this function if there is a chance that IT will call it as well
      // __disable_irq(); // make sure TX_in_Progress is mutexed
      if(!TX_in_Progress && !InError) {
        // __enable_irq();

        static const uint8_t *p;
        static size_t Sz;
        if(p != nullptr) Send((uint8_t *)p,Sz); // sending what was not sent the last time
        else {
          // if I am trying to transmit single characters I will pack them into this Buffer and transmit all at once, it
          // will be much more efficient
          constexpr uint16_t BufferSize = 256; // just a meaningful number of characters to make transfer more efficient
          static uint8_t Buffer[BufferSize]; // static because is referred to after function exits
          uint16_t NumInBuffer = 0;

          while((*pGetBlockToSend)(&p,&Sz)) { // pGetBlockToSend can return a sequence of bytes one by one, and transmitting them would be very
            // inefficient, so let's buffer them while Sz is 1.
            if(Sz == 1) {
              Buffer[NumInBuffer++] = *p;
              if(NumInBuffer == BufferSize) break;
            } else { // Ok, we got a block with Sz > 1
              if(NumInBuffer) { // sending what we accumulated in the buffer and remembering the block for the next time
                Send(Buffer, NumInBuffer);
                return; // it is return here because I want to keep "p", the block will be sent next time
              } else {
                Send((uint8_t *)p,Sz);
                break;
              }
            }
          } // while

          if(NumInBuffer) Send(Buffer, NumInBuffer);
        }
        p = nullptr;
      }
      // __enable_irq();
    } // TryToSend

    static void TX_Done() { // this thing is called from inside HAL_UART_TxCpltCallback
      TX_in_Progress = false;
      // TryToSend(); // breaks things
    } // TX_Done

    static void RX_Callback() {
      (*pStoreReceivedByte)(NewByte);
      RX_Byte_IT();
    } // RX_Callback

    static void ErrorCallback(uint32_t ErrorCode) { // we recover from the error in Protocol::cycle
      InError = true;
      snprintf(ErrStr,ERR_STR_MAX_SZ,"UART error code = %lu\n", ErrorCode);
    } // ErrorCallback

    static const char *GetError() {
      if(InError) {
        InError = false;
        return ErrStr;
      } else return nullptr;
    } // GetError

    static void FlushRX() {} // RX is interrupt driven, nothing to flush
  }; // UART_for_Port


#define U UART_for_Port<puart>

  T tUART_Link U::UART_Link(puart, U::TX_Done, U::ErrorCallback, U::RX_Callback);

  T tStoreReceivedByte U::pStoreReceivedByte;
  T tGetBlockToSend U::pGetBlockToSend;
  T uint8_t U::NewByte;
  T bool U::TX_in_Progress = false;
// T typename U::ErrorCodes_ U::Error = NO_ERROR;
  T char U::ErrStr[];
  T bool U::InError = false;


#undef T
#undef U

}; // namespace avp
#endif /* UART1_HPP_INCLUDED */
