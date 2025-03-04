#pragma once

/**

Reserve space in FLASH for simulated eeprom by sectors.
@note We can not rewrite bytes, after a byte is written the whole sector has to be erased before rewriting.
e.g. in *.ld file:
MEMORY
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 254K // 256K - 2K
  EEPROM (rwx)    : ORIGIN = 0x0803F800, LENGTH = 2K   // Last 2 KB sector 127
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 40K  // Adjust per model
  CCMRAM (rw)     : ORIGIN = 0x10000000, LENGTH = 8K
}

_flash_start = ORIGIN(FLASH);
_flash_size  = LENGTH(FLASH);
_eeprom_start = ORIGIN(EEPROM);
_eeprom_size  = LENGTH(EEPROM);
*/

#include "stm32f3xx_hal.h"

namespace flash {
  void erase_sector(uint32_t sector);
  void write(uint32_t* addr, uint32_t data);

//  uint32_t flash_read(uint32_t* addr) {
//    return *addr;
//  }
} // namespace flash

/**
 * abstract class, should be inherited by a single variable stored in the EEPROM
 * @note Data should be aligned to uint32 either by using union or alignas(4) (sinse c++11)
 */
struct StoredInEEPROM {
  StoredInEEPROM() { Read(); }
  virtual uint32_t DataSize() = 0;
  virtual uint32_t *DataStart() = 0; // Data should be aligned
  void Write();
  void Read();
}; //  StoredInEEPROM


