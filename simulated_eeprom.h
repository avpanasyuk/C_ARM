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

#ifdef __cplusplus
extern "C" {
#endif

void EEPROM_Write(const uint32_t *pDWORD, uint32_t nDWORDs);
bool EEPROM_Read(uint32_t *pDWORD, uint32_t nDWORDs);

#ifdef __cplusplus
}
#endif



