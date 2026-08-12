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
#if defined(STM32F1)
  // On the GD32F303 (an STM32F1 clone) the ST HAL's HAL_FLASHEx_Erase returns HAL_ERROR and leaves
  // the page intact, even though the silicon erases fine (e.g. EBlink erases it). Drive the FMC
  // erase registers directly instead - the PER/AR/STRT page-erase sequence is identical on real
  // STM32F1/F3. HAL_FLASH_Program (below) works on the GD32, so only the erase is bypassed.
  for(uint32_t i = 0; i < Npages; ++i) {
    if(FLASH->CR & FLASH_CR_LOCK) { FLASH->KEYR = 0x45670123u; FLASH->KEYR = 0xCDEF89ABu; }
    while(FLASH->SR & FLASH_SR_BSY) {}
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR; // write-1-to-clear stale flags
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR  = page + i * FLASH_PAGE_SIZE;
    FLASH->CR |= FLASH_CR_STRT;
    while(FLASH->SR & FLASH_SR_BSY) {}
    FLASH->CR &= ~FLASH_CR_PER;
  }
#else
  FLASH_EraseInitTypeDef eraseInit = { FLASH_TYPEERASE_PAGES, page, Npages };
  uint32_t pageError;
  AVP_ASSERT(HAL_FLASHEx_Erase(&eraseInit, &pageError) == HAL_OK);
#endif
  const uint32_t *end = (const uint32_t *)(page + Npages * FLASH_PAGE_SIZE);
  for(const uint32_t *p = (const uint32_t *)page; p < end; ++p)
    AVP_ASSERT(*p == 0xFFFFFFFFu); // verify the page is blank (catches a genuine erase failure)
} // erase_page

/**
 * After the page is erased every DWORD can be written once only, another erase should happen before rewrite
 */
static void flash_write(uint32_t* addr, uint32_t data) {
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)addr, data);
  AVP_ASSERT(*addr == data); // verify by readback (don't trust the GD32's HAL status)
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

