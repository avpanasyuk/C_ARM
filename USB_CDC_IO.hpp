#ifndef USB_CDC_IO_HPP_INCLUDED
#define USB_CDC_IO_HPP_INCLUDED

/**
  Used for "Port" class, so
     -# should be static
      -# static void TryToSend(); - this class calls it to let HW_IO_ know that there are
        new data in buffer to transmit. This function may be called at the end of all primary "write"
        functions, from the interrupt indicating end of previous transfer, or from the "cycle" loop
        just in case. It should maintain locks when necessary.
      -# should provide static void FlushRX();
      -# should provide const char *GetError();
      -# should provide RX_Byte_IT() to restart RX if it stalled
       - static void SetCallBacks(tStoreReceivedByte pStoreReceivedByte_,  tGetBlockToSend pGetBlockToSend_)
        - bool StoreReceivedByte(uint8_t b)
        - bool GetBlockToSend(const uint8_t **p, size_t *pSz)
      -# HW_IO_ should call StoreReceivedByte supplied to it by SetCallBacks call when it received a byte
      -# HW_IO_ should call GetBlockToSend when it is ready to send new data
*/
#include "../C_General/IO.h"

namespace avp {
  struct USB_CDC_IO {
    static void TryToSend();

    static const char *GetError() { return nullptr; }
    static void FlushRX();
    static void RX_Byte_IT() {};
    static void SetCallBacks(tStoreReceivedByte pStoreReceivedByte_,  tGetBlockToSend pGetBlockToSend_);
  }; // class USB_CDC_IO
} // namespace avp

#endif /* USB_CDC_IO_HPP_INCLUDED */
