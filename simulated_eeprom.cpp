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
#include "../C_ARM/Error.h"
#include "../C_General/MyMath.h"

static constexpr uint32_t FLASH_SECTOR_SIZE = 2048;

// Declare linker symbols
extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _eeprom_start;
extern uint32_t _eeprom_size; // the same size as FLASH sector, code has to be modified if need more

uint32_t *flash_start = &_flash_start;
uint32_t flash_size = (uint32_t)&_flash_size;
uint32_t *eeprom_start = &_eeprom_start;
uint32_t eeprom_size = (uint32_t)&_eeprom_size; // the same size as FLASH sector, code has to be modified if need more

namespace flash {
  void erase_sector(uint32_t sector) {
    FLASH_EraseInitTypeDef eraseInit = {
      .TypeErase = FLASH_TYPEERASE_SECTORS,
      .Sector = sector,
      .NbSectors = 1,
      .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };
    uint32_t sectorError;
    AVP_ASSERT(HAL_FLASHEx_Erase(&eraseInit, &sectorError) == HAL_OK);
  } // erase_sector

  void write(uint32_t* addr, uint32_t data) {
    AVP_ASSERT(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)addr, data) == HAL_OK);
  }
} // namespace flash

void StoredInEEPROM::Write() {
  AVP_ASSERT(DataSize() <= eeprom_size);
  AVP_ASSERT(HAL_FLASH_Unlock() == HAL_OK);
  flash::erase_sector((uint32_t)eeprom_start);
  uint32_t count = avp::CeilRatio(DataSize(), sizeof(uint32_t));
  uint32_t *p_eeprom = eeprom_start;
  const uint32_t *p = DataStart();
  while(count--) flash::write(p_eeprom++, *(p++));
  AVP_ASSERT(HAL_FLASH_Lock() == HAL_OK);
} // StoredInEEPROM::Write

void StoredInEEPROM::Read() {
  uint32_t count = avp::CeilRatio(DataSize(), sizeof(uint32_t));
  uint32_t *p_eeprom = eeprom_start;
  const uint32_t *p = DataStart();
  while(count--) *(p++) = (**p_eeprom++);
} // StoredInEEPROM::Read

