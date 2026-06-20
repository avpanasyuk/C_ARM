/**

Reserve space in FLASH for simulated eeprom by pages.
@note We can not rewrite bytes, after a byte is written the whole page has to be erased before rewriting.
e.g. in *.ld file:
MEMORY
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 254K // 256K - 2K
  EEPROM (rwx)    : ORIGIN = 0x0803F800, LENGTH = 2K   // Last 2 KB page 127
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 40K  // Adjust per model
  CCMRAM (rw)     : ORIGIN = 0x10000000, LENGTH = 8K
}

_flash_start = ORIGIN(FLASH);
_flash_size  = LENGTH(FLASH);
_eeprom_start = ORIGIN(EEPROM);
_eeprom_size  = LENGTH(EEPROM);
*/

/// @cond
#include "MCU_HAL.h" // umbrella: pulls in hal_def (HAL_StatusTypeDef) before the FLASH module
/// @endcond
#include "../C_General/General.hpp"
#include "../C_General/MyMath.hpp"
#include "../C_ARM/Error.h"

// Declare linker symbols
extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _eeprom_start;
extern uint32_t _eeprom_size; // the same size as FLASH page, code has to be modified if need more

uint32_t *flash_start = &_flash_start;
uint32_t flash_size = (uint32_t)&_flash_size;
uint32_t *eeprom_start = &_eeprom_start;
uint32_t eeprom_size = (uint32_t)&_eeprom_size; // the same size as FLASH page, code has to be modified if need more

static void flash_erase_page(uint32_t page, uint32_t Npages) {
  FLASH_EraseInitTypeDef eraseInit = { FLASH_TYPEERASE_PAGES, page, Npages };
  uint32_t pageError;
  AVP_ASSERT(HAL_FLASHEx_Erase(&eraseInit, &pageError) == HAL_OK);
} // erase_page

/**
 * After the page is erased every DWORD can be written once only, another erase should happen before rewrite
 */
static void flash_write(uint32_t* addr, uint32_t data) {
  AVP_ASSERT(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)addr, data) == HAL_OK);
} // flash_write

/**
 * The headache is that we can write only DWORDs to EEPROM, so
 * @param p should point to a struct aligned to 4 bytes using __attribute__((aligned(4))) or alignas(x).
 * @param Nbytes:The sizeof of such structure is always divisible by 4
 */
void EEPROM_Write(const uint32_t *p, uint32_t Nbytes) {
  AVP_ASSERT(Nbytes % sizeof(uint32_t) == 0);
  AVP_ASSERT(Nbytes + sizeof(uint32_t) <= eeprom_size); // another DWORD for CRC
  AVP_ASSERT(HAL_FLASH_Unlock() == HAL_OK);
  flash_erase_page((uint32_t)eeprom_start, avp::CeilRatio<uint32_t>(Nbytes + sizeof(uint32_t), FLASH_PAGE_SIZE));
  uint32_t CRC_val = avp::Crc16((const uint8_t *)p, Nbytes,0);
  uint32_t *p_eeprom = eeprom_start;
  uint32_t count = Nbytes/sizeof(uint32_t);
  while(count--) flash_write(p_eeprom++, *(p++));
  flash_write(p_eeprom, CRC_val);
  AVP_ASSERT(HAL_FLASH_Lock() == HAL_OK);
} // StoredInEEPROM::Write

/**
 * if CRC in EEPROM is wrong (usually because nothing was written yet) the function returns false and nothing
 * is written to *p
 * @param p should point to aligned variable, used union, __attribute__((aligned(x))) or better alignas(x)
 * @param Nbytes:The sizeof of such structure is always divisible by 4
 * @retval Error string or NULL if no error
 */
const char *EEPROM_Read(uint32_t *p, uint32_t Nbytes) {
  AVP_ASSERT(Nbytes % sizeof(uint32_t) == 0);
  AVP_ASSERT(Nbytes + sizeof(uint32_t) <= eeprom_size); // another DWORD for CRC
  uint32_t CRC_val = avp::Crc16((const uint8_t *)eeprom_start, Nbytes, 0);
  uint32_t count = Nbytes/sizeof(uint32_t);
  if(CRC_val != *(eeprom_start + count)) return "Bad CRC!";

  uint32_t *p_eeprom = eeprom_start;
  while(count--) *(p++) = *(p_eeprom++);
  return NULL;
} // StoredInEEPROM::Read

