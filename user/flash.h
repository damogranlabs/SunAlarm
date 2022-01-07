#ifndef __FLASH_H
#define __FLASH_H

#include <stdbool.h>

void flash_unlock(void);
void flash_lock(void);

bool is_cfg_data_in_flash(void);
void flash_read(void);
void flash_write(void);

#endif // __FLASH_H