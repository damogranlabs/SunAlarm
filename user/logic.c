#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "stm32f0xx_hal_flash.h"
#include "rtc.h"

#include "lcd.h"
#include "buttons.h"
#include "rot_enc.h"

#include "logic.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"

void _set_alarm_defaults(void);
void _set_alarm_start_time(void);
void _handle_alarm_time(void);

void _handle_setup(bool force_refresh);
void _handle_setup_alarm_time(void);
void _handle_setup_time(bool force_refresh, bool h_or_m);
void _handle_setup_sun_intensity(bool force_refresh, bool change_min_intensity);
void _handle_setup_sun_manual_intensity(bool force_refresh);
void _handle_setup_wakeup_time(bool force_refresh);

void _manipulate_time(uint8_t *h_ptr, uint8_t *m_ptr, int16_t change_min);

uint32_t _get_alarm_sun_intensity_msec_per_step(void);
uint32_t _get_next_alarm_sun_intensity(bool reset);

sm_area_t sm_area;
configuration_t cfg_data;
runtime_data_t runtime_data;
extern rot_enc_data_t encoder;

#define CFG_DATA_FLASH_ADD 0x80003C00 //0x80005000

void _set_alarm_defaults(void)
{
  cfg_data.is_alarm_enabled = false;
  cfg_data.alarm_time[H_POS] = DEFAULT_ALARM_TIME_H;
  cfg_data.alarm_time[M_POS] = DEFAULT_ALARM_TIME_M;
  cfg_data.wakeup_time_min = DEFAULT_WAKEUP_TIME_MIN;
  cfg_data.sun_intensity_min = DEFAULT_SUN_INTENSITY_MIN;
  cfg_data.sun_intensity_max = DEFAULT_SUN_INTENSITY_MAX;
  cfg_data.sun_manual_intensity = DEFAULT_SUN_INTENSITY;
}

void set_defaults(void)
{
  runtime_data.is_setup_mode = false;
  runtime_data.is_alarm_active = false;
  runtime_data.current_alarm_sun_intensity = cfg_data.sun_intensity_min;
  runtime_data.next_alarm_sun_intensity_timestamp = 0;

  _set_alarm_defaults();
  _set_alarm_start_time();
}

void set_setup_mode(bool is_enabled)
{
  runtime_data.is_setup_mode = is_enabled;
  if (is_enabled)
  {
    sm_area = SETUP_WAKEUP_TIME;
    rot_enc_reset_count(&encoder);

    get_current_time(&cfg_data.time[H_POS], &cfg_data.time[M_POS], NULL);

    sun_set_intensity(cfg_data.sun_intensity_min);
    sun_pwr_on();

    _handle_setup(true);
  }
  else
  {
    // TODO save cfg settings to flash?
    sun_pwr_off();

    set_new_time(cfg_data.time[H_POS], cfg_data.time[M_POS], 0);

    _set_alarm_start_time();

    show_default();
  }
}

bool is_setup_mode(void)
{
  return runtime_data.is_setup_mode;
}

void update_settings(void)
{
  sm_area++;
  if (sm_area >= SETUP_DONE)
  {
    sm_area = SETUP_WAKEUP_TIME;
  }

  rot_enc_reset_count(&encoder);

  _handle_setup(true);
}

void handle_interactions(void)
{
  if (is_setup_mode())
  {
    _handle_setup(false);
  }
  else if (is_alarm_enabled())
  {
    // alarm time is updated here via rotary encoder, while alarm state (on/off) is toggled and handled via button
    _handle_alarm_time();
  }
}

void _set_alarm_start_time(void)
{
  runtime_data.alarm_start_time[H_POS] = cfg_data.alarm_time[H_POS];
  runtime_data.alarm_start_time[M_POS] = cfg_data.alarm_time[M_POS];
  _manipulate_time(&runtime_data.alarm_start_time[H_POS],
                   &runtime_data.alarm_start_time[M_POS],
                   -cfg_data.wakeup_time_min);
}

void handle_alarm(void)
{
  uint8_t h, m, s;

  if (!is_alarm_enabled())
    return;

  if (is_setup_mode())
    return;

  get_current_time(&h, &m, &s);

  if (is_alarm_active())
  {
    // alarm is already in progress
    // only handle alarm per minute basis
    if (s == 0)
    {
      if (h == cfg_data.alarm_time[H_POS])
      {
        if (m == cfg_data.alarm_time[M_POS])
        {
          // finish alarm
          set_alarm_active(false);

          // do not power off. just let the user turn the sun/alarm off.
          return;
        }
      }
    }
    else if (HAL_GetTick() >= runtime_data.next_alarm_sun_intensity_timestamp)
    {
      runtime_data.next_alarm_sun_intensity_timestamp += _get_alarm_sun_intensity_msec_per_step();
      runtime_data.current_alarm_sun_intensity = _get_next_alarm_sun_intensity(false);
      sun_set_intensity_precise(runtime_data.current_alarm_sun_intensity);

      return;
    }
  }
  else
  {
    // check if it is alarm time
    // only handle alarm per minute basis
    if (s == 0)
    {
      if (h == runtime_data.alarm_start_time[H_POS])
      {
        if (m == runtime_data.alarm_start_time[M_POS])
        {
          // its is wake up time!
          runtime_data.next_alarm_sun_intensity_timestamp = HAL_GetTick() + _get_alarm_sun_intensity_msec_per_step();
          runtime_data.current_alarm_sun_intensity = _get_next_alarm_sun_intensity(true);

          set_alarm_active(true);
          sun_set_intensity_precise(runtime_data.current_alarm_sun_intensity);
          sun_pwr_on();
        }
      }
    }
  }
}

uint32_t _get_alarm_sun_intensity_msec_per_step(void)
{
  uint32_t num_of_steps = (get_sun_intensity_resolution() / SUN_INTENSITY_MAX) * (uint32_t)(cfg_data.sun_intensity_max - cfg_data.sun_intensity_min);

  //60000 = 60 * 1000 (min -> sec -> sec -> msec)
  uint32_t msec_per_step = (((uint32_t)cfg_data.wakeup_time_min) * 60000) / num_of_steps;

  // At very low wakeup times, it might be that msec_per_step is lower than 1.
  // In that case, just set it to 1 step per msec
  if (!msec_per_step)
  {
    msec_per_step = 1;
  }

  return msec_per_step;
}

uint32_t _get_next_alarm_sun_intensity(bool reset)
{
  static uint32_t intensity = 0;

  if (reset)
  {
    intensity = (uint32_t)cfg_data.sun_intensity_min;
  }
  else
  {
    intensity++;
  }

  return intensity;
}

void _handle_setup(bool force_refresh)
{
  switch (sm_area)
  {
  case SETUP_WAKEUP_TIME:
    _handle_setup_wakeup_time(force_refresh);
    break;

  case SETUP_SUN_MIN_INTENSITY:
    _handle_setup_sun_intensity(force_refresh, true);
    break;
  case SETUP_SUN_MAX_INTENSITY:
    _handle_setup_sun_intensity(force_refresh, false);
    break;

  case SETUP_SUN_DEFAULT_INTENSITY:
    _handle_setup_sun_manual_intensity(force_refresh);
    break;

  case SETUP_TIME_H:
    _handle_setup_time(force_refresh, true);
    break;
  case SETUP_TIME_M:
    _handle_setup_time(force_refresh, false);
    break;

  default:
    Error_Handler();
  }
}

void set_alarm_state(bool is_enabled)
{
  cfg_data.is_alarm_enabled = is_enabled;

  show_alarm_state();
}

bool is_alarm_enabled(void)
{
  return cfg_data.is_alarm_enabled;
}

bool is_alarm_active(void)
{
  return runtime_data.is_alarm_active;
}

void set_alarm_active(bool is_active)
{
  runtime_data.is_alarm_active = is_active;
}

// modify alarm settings in the runtime
void _handle_alarm_time(void)
{
  int16_t count = rot_enc_get_count(&encoder);
  // alarm is set in minutes
  if (count != 0)
  {
    _manipulate_time(&cfg_data.alarm_time[H_POS], &cfg_data.alarm_time[M_POS], count);
    _set_alarm_start_time();

    show_alarm_state();
    ctrl_lcd_backlight(true, true);
  }
}

void _handle_setup_wakeup_time(bool force_refresh)
{
  char time_str[TIME_STR_SIZE];

  int32_t count = rot_enc_get_count(&encoder);
  if ((count != 0) || (force_refresh == true))
  {
    if ((cfg_data.wakeup_time_min + count) > 255)
    {
      cfg_data.wakeup_time_min = 255;
    }
    else if ((cfg_data.wakeup_time_min + count) < 0)
    {
      cfg_data.wakeup_time_min = 0;
    }
    else
    {
      cfg_data.wakeup_time_min += count;
    }
    sprintf(time_str, "%d", cfg_data.wakeup_time_min);
    show_setup_item(sm_area, &time_str[0]);
  }
}

void _handle_setup_time(bool force_refresh, bool change_hours)
{
  //change_hours: if True, rotary encoder is setting hours, else minutes
  char time_str[TIME_STR_SIZE];
  int8_t count = rot_enc_get_count(&encoder);

  if ((count != 0) || (force_refresh == true))
  {
    if (change_hours)
    {
      count += cfg_data.time[H_POS];
      while (count > 23)
      {
        count -= 24;
      }
      while (count < 0)
      {
        count += 24;
      }
      cfg_data.time[H_POS] = abs(count);
    }
    else
    {
      count += cfg_data.time[M_POS];
      while (count > 59)
      {
        count -= 60;
      }
      while (count < 0)
      {
        count += 60;
      }
      cfg_data.time[M_POS] = abs(count);
    }

    time_to_str(time_str, cfg_data.time[H_POS], cfg_data.time[M_POS], -1);
    show_setup_item(sm_area, time_str);
  }
}

void _handle_setup_sun_intensity(bool force_refresh, bool change_min_intensity)
{
  //change_min_intensity: if True, rotary encoder is setting minimum intensity,
  // else max intensity
  char time_str[3]; // 0 - SUN_INTENSITY_MAX
  int8_t count = rot_enc_get_count(&encoder);

  if ((count != 0) || (force_refresh == true))
  {
    if (change_min_intensity)
    {
      if ((cfg_data.sun_intensity_min + count) > cfg_data.sun_intensity_max)
      {
        cfg_data.sun_intensity_min = cfg_data.sun_intensity_max - 1;
      }
      else if ((cfg_data.sun_intensity_min + count) <= 0)
      {
        cfg_data.sun_intensity_min = 0;
      }
      else
      {
        cfg_data.sun_intensity_min += count;
      }
      sun_set_intensity(cfg_data.sun_intensity_min);
      sprintf(time_str, "%d", cfg_data.sun_intensity_min);
    }
    else
    {
      if ((cfg_data.sun_intensity_max + count) > SUN_INTENSITY_MAX)
      {
        cfg_data.sun_intensity_max = SUN_INTENSITY_MAX;
      }
      else if ((cfg_data.sun_intensity_max + count) <= cfg_data.sun_intensity_min)
      {
        cfg_data.sun_intensity_max = cfg_data.sun_intensity_min + 1;
      }
      else
      {
        cfg_data.sun_intensity_max += count;
      }
      sun_set_intensity(cfg_data.sun_intensity_max);
      sprintf(time_str, "%d", cfg_data.sun_intensity_max);
    }
    show_setup_item(sm_area, time_str);
  }
}

void _handle_setup_sun_manual_intensity(bool force_refresh)
{
  char val_str[3]; // 0 - SUN_INTENSITY_MAX
  int16_t count = rot_enc_get_count(&encoder);
  uint16_t value;

  if ((count != 0) || (force_refresh == true))
  {
    value = (uint16_t)cfg_data.sun_manual_intensity + count;

    if (value > SUN_INTENSITY_MAX)
    {
      cfg_data.sun_manual_intensity = SUN_INTENSITY_MAX;
    }
    else if (value <= 0)
    {
      cfg_data.sun_manual_intensity = 0;
    }
    else
    {
      cfg_data.sun_manual_intensity = (uint8_t)value;
    }

    sun_set_intensity(cfg_data.sun_manual_intensity);
    sprintf(val_str, "%d", cfg_data.sun_manual_intensity);

    show_setup_item(sm_area, val_str);
  }
}

void _manipulate_time(uint8_t *h_ptr, uint8_t *m_ptr, int16_t change_min)
{
  int16_t h = (int16_t)*h_ptr;
  int16_t m = (int16_t)*m_ptr;

  m += change_min;
  if (m > 0)
  {
    while (m >= 60)
    {
      m = m - 60;
      h++;
      if (h == 24)
      {
        h = 0;
      }
    }
  }
  else
  {
    while (m < 0)
    {
      m = m + 60;
      h--;
      if (h < 0)
      {
        h = 23;
      }
    }
  }

  *h_ptr = (uint8_t)h;
  *m_ptr = (uint8_t)m;
}

void save_settings(void)
{
  uint8_t var_idx;
  uint32_t *cfg_data_ptr = (uint32_t *)&cfg_data;
  uint32_t flash_add = CFG_DATA_FLASH_ADD;

  volatile uint8_t wrBuf[5] = {0x11, 0x22, 0x33, 0x44, 0x55};

  __disable_irq();
  uint32_t flash_page_err = 0;
  FLASH_EraseInitTypeDef pEraseInit = {0};
  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.PageAddress = CFG_DATA_FLASH_ADD;
  pEraseInit.NbPages = 1;

  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0)
  {
  }
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&pEraseInit, &flash_page_err);
  HAL_FLASH_Lock();

  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0)
  {
  }
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

  volatile HAL_StatusTypeDef status;
  status = HAL_FLASH_Unlock();
  // LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  // LL_FLASH_DisablePrefetch();
  // HAL_FLASH_OB_Unlock();
  //for (uint32_t i = 0; i < 5; i++)
  //{
  //  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_add, ((uint8_t *)wrBuf)[i]);
  //  flash_add++;
  //}

  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0)
  {
  }
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

  uint16_t data = 11234;

  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_add, &data);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_add + 4, *cfg_data_ptr + 4);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_add + 8, *cfg_data_ptr + 8);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_add + 12, *cfg_data_ptr + 12);

  for (var_idx = 0; var_idx < sizeof(cfg_data); var_idx += 4, cfg_data_ptr++, flash_add += 4)
  {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_add, *cfg_data_ptr);
  }
  HAL_FLASH_Lock();

  __enable_irq();
}

void read_settings(void)
{
  uint8_t var_idx;
  uint32_t *cfg_data_ptr = (uint32_t *)&cfg_data;
  uint32_t flash_address = CFG_DATA_FLASH_ADD;

  for (var_idx = 0; var_idx < sizeof(cfg_data); var_idx += 4, cfg_data_ptr++, flash_address += 4)
  {
    *cfg_data_ptr = *(uint32_t *)flash_address;
  }
}