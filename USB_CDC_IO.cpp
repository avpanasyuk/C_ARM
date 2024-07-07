#include "../C_General/Error.h"
#include "../C_ARM/USB_CDC_IO.hpp"

const uint8_t *avp::USB_CDC_IO::pBlockBeingSent = nullptr;
size_t avp::USB_CDC_IO::LengthOfBlockBeingSent = 0;
avp::tStoreReceivedByte avp::USB_CDC_IO::pStoreReceivedByte = nullptr;
avp::tGetBlockToSend avp::USB_CDC_IO::pGetBlockToSend = nullptr;

void avp_USB_CDC_IO_ReceivedBlock(uint8_t* Buf, uint32_t *Len) {
  AVP_ASSERT(avp::USB_CDC_IO::pStoreReceivedByte != nullptr);
  while((*Len)--) avp::USB_CDC_IO::pStoreReceivedByte(*(Buf++));
} // ReceivedBlock
