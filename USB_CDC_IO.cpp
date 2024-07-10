#include <tusb.h>
#include "../C_General/Error.h"
#include "../C_ARM/USB_CDC_IO.hpp"

static const uint8_t *pBlockBeingSent;
static size_t LengthOfBlockBeingSent;
static avp::tStoreReceivedByte pStoreReceivedByte;
static avp::tGetBlockToSend pGetBlockToSend;

void avp::USB_CDC_IO::SetCallBacks(tStoreReceivedByte pStoreReceivedByte_,  tGetBlockToSend pGetBlockToSend_) {
  pStoreReceivedByte = pStoreReceivedByte_;
  pGetBlockToSend = pGetBlockToSend_;
} // SetCallBacks


#ifdef CFG_TUD_ENABLED // using tinyusb library
#include <cdc_device.h>

void avp::USB_CDC_IO::FlushRX() {
  tud_cdc_read_flush();
} // FlushRX

// Invoked when received new data
void tud_cdc_rx_cb(uint8_t) {
  int32_t Char;
  while(1) {
    Char = tud_cdc_read_char();
    if(Char < 0) break;
    pStoreReceivedByte(uint8_t(Char));
  }
} // tud_cdc_rx_cb

// Invoked when a TX is complete and therefore space becomes available in TX buffer
void tud_cdc_tx_complete_cb(uint8_t) {
  if(pBlockBeingSent == nullptr)
    pGetBlockToSend(&pBlockBeingSent, &LengthOfBlockBeingSent);
  if(pBlockBeingSent != nullptr) {
    auto WrittenSize = tud_cdc_write((uint8_t *)pBlockBeingSent, LengthOfBlockBeingSent);
    if(WrittenSize == LengthOfBlockBeingSent)
      pGetBlockToSend(&pBlockBeingSent, &LengthOfBlockBeingSent);
    else {
      AVP_ASSERT(WrittenSize < LengthOfBlockBeingSent);
      pBlockBeingSent +=  WrittenSize;
      LengthOfBlockBeingSent -= WrittenSize;
    }
  }
} //tud_cdc_tx_complete_cb

void avp::USB_CDC_IO::TryToSend() { tud_cdc_tx_complete_cb(0); }

#else
#include <usbd_cdc_if.h> // STM32 MX implementation

void avp::USB_CDC_IO::TryToSend() {
  // FIXME: should there be a mutex for pBlockBeingSent
  if(pBlockBeingSent == nullptr)
    pGetBlockToSend(&pBlockBeingSent, &LengthOfBlockBeingSent);
  if(pBlockBeingSent != nullptr) {
    if(CDC_Transmit_FS((uint8_t *)pBlockBeingSent, LengthOfBlockBeingSent) == USBD_OK)
      pGetBlockToSend(&pBlockBeingSent, &LengthOfBlockBeingSent);
  }
} // TryToSend

void avp::USB_CDC_IO::FlushRX() {};

void avp_USB_CDC_IO_ReceivedBlock(uint8_t* Buf, uint32_t *Len) {
  AVP_ASSERT(avp::USB_CDC_IO::pStoreReceivedByte != nullptr);
  while((*Len)--) avp::USB_CDC_IO::pStoreReceivedByte(*(Buf++));
} // ReceivedBlock
#endif

