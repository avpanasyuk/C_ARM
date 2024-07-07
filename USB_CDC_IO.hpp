#ifndef USB_CDC_IO_HPP_INCLUDED
#define USB_CDC_IO_HPP_INCLUDED

#include <usbd_cdc_if.h>

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
#ifdef __cplusplus
#include "../C_General/IO.h"

namespace avp {
  struct USB_CDC_IO {
    static const uint8_t *pBlockBeingSent;
    static size_t LengthOfBlockBeingSent;
    static tStoreReceivedByte pStoreReceivedByte;
    static tGetBlockToSend pGetBlockToSend;

    static inline void TryToSend() {
      // FIXME: should there be a mutex for pBlockBeingSent
      if(pBlockBeingSent != nullptr) {
        if(CDC_Transmit_FS((uint8_t *)pBlockBeingSent, LengthOfBlockBeingSent) == USBD_OK)
           pGetBlockToSend(&pBlockBeingSent, &LengthOfBlockBeingSent);
      }
    } // TryToSend

    static const char *GetError() { return nullptr; }
    static void FlushRX() {};
    static void RX_Byte_IT() {};
    static void SetCallBacks(tStoreReceivedByte pStoreReceivedByte_,  tGetBlockToSend pGetBlockToSend_) {
      pStoreReceivedByte = pStoreReceivedByte_;
      pGetBlockToSend = pGetBlockToSend;
    } // SetCallBacks
  }; // class USB_CDC_IO
} // namespace avp

extern "C"
#endif
void avp_USB_CDC_IO_ReceivedBlock(uint8_t* Buf, uint32_t *Len);

#endif /* USB_CDC_IO_HPP_INCLUDED */
