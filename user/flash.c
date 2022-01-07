#include "stdint.h"

#include "stm32f0xx.h"
#include "main.h"

#include "flash.h"
#include "logic.h"

extern const uint32_t FLASH_LENGTH;
extern const uint32_t END_OF_FLASH;

#define FLASH_PAGE_SIZE 0x400 // 1K

void _flash_write(uint32_t add, uint32_t data);
void _flash_erase(uint32_t add);
void _wait_last_flash_operation(void);
uint32_t _get_rw_add(void);

extern configuration_t cfg_data;

void flash_unlock(void)
{
  _wait_last_flash_operation();

  // based on `HAL_StatusTypeDef HAL_FLASH_Unlock(void)`
  if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != RESET)
  {
    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    /* Verify Flash is unlocked */
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != RESET)
    {
      Error_Handler();
    }
  }

  _wait_last_flash_operation();
}

void flash_lock(void)
{
  //based on `HAL_StatusTypeDef HAL_FLASH_Lock(void)`

  // Set the LOCK Bit to lock the FLASH Registers access
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);
  _wait_last_flash_operation();
}

void _wait_last_flash_operation(void)
{
  // based on `HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout)`
  while ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY)
  {
  }
}

void _flash_write(uint32_t add, uint32_t data)
{
  /* based on `HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)`
  Where TypeProgram: #define FLASH_TYPEPROGRAM_WORD       (0x02U)  // !<Program a word (32-bit) at a specified address.
  */
  uint8_t idx = 0U;

  // idx < 2: Program word (32-bit = 2*16-bit) at a specified address.
  for (idx = 0U; idx < 2; idx++)
  {
    // based on `FLASH_Program_HalfWord((add + (2U * index)), (uint16_t)(data >> (16U * index)));`
    SET_BIT(FLASH->CR, FLASH_CR_PG);
    // Write data in the address
    *(__IO uint16_t *)(add + (2U * idx)) = (uint16_t)(data >> (16U * idx));

    // Wait for last operation to be completed
    _wait_last_flash_operation();

    // If the program operation is completed, disable the PG Bit
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
  }
}

void _flash_erase(uint32_t add)
{
  // based on `HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError)`
  uint32_t address = 0;
  uint32_t page_num = 1;

  // Page Erase requested on address located on bank1
  // Erase page by page to be done
  for (address = add; address < (add + (page_num * FLASH_PAGE_SIZE)); address += FLASH_PAGE_SIZE)
  {
    //FLASH_PageErase(address);
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    WRITE_REG(FLASH->AR, address);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);

    _wait_last_flash_operation();

    // If the erase operation is completed, disable the PER Bit
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
  }
}

uint32_t _get_rw_add(void)
{
  return ((uint32_t)(&END_OF_FLASH)) - FLASH_PAGE_SIZE;
}

void flash_write(void)
{
  uint32_t dst_add = _get_rw_add();
  uint32_t *src_add = (uint32_t *)&cfg_data;
  uint8_t idx;

  flash_unlock();
  _flash_erase(dst_add);

  // write in chunks of 32 bits (4 bytes)
  // Note: cfg struct is automatically padded by compiler
  for (idx = 0; idx < sizeof(cfg_data); idx += 4, src_add++, dst_add += 4)
  {
    _flash_write(dst_add, *src_add);
  }

  flash_lock();
}

void flash_read(void)
{
  uint32_t src_add = _get_rw_add();
  uint32_t *dst_add = (uint32_t *)&cfg_data;
  uint8_t idx = 0;

  // read in chunks of 32 bits (4 bytes)
  for (idx = 0; idx < sizeof(cfg_data); idx += 4, dst_add++, src_add += 4)
  {
    *dst_add = *((uint32_t *)src_add);
  }
}

bool is_cfg_data_in_flash(void)
{
  uint32_t src_add = _get_rw_add();

  if (*((uint32_t *)src_add) == 0xFFFFFFFF)
  {
    return false;
  }
  else
  {
    return true;
  }
}