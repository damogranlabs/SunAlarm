/*
 * setup_menu.c
 *
 *  Created on: Feb 14, 2021
 *      Author: domen
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lcd.h"
#include "buttons.h"
#include "rot_enc.h"

#include "main.h"
#include "setup_menu.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"

void _show_alarm_state(void);
void _set_alarm_defaults(void);
void _set_alarm_start_time(void);
void _handle_alarm_time(void);

void _handle_setup(bool force_refresh);
void _handle_setup_alarm_time(void);
void _handle_setup_time(bool force_refresh, bool h_or_m);
void _handle_setup_sun_intensity(bool force_refresh, bool change_min_intensity);
void _handle_setup_sun_manual_intensity(bool force_refresh);
void _handle_setup_wakeup_time(bool force_refresh);
void _handle_setup_music(bool force_refresh);

void _manipulate_time(uint8_t *h_ptr, uint8_t *m_ptr, int16_t change_min);

uint32_t _get_alarm_sun_intensity_msec_per_step(void);
uint32_t _get_next_alarm_sun_intensity(bool reset);

sm_area_t sm_area;
configuration_t cfg_data;
runtime_data_t runtime_data;

extern rot_enc_data_t encoder;

void _clear_lcd_sm_area(void);
void _clear_lcd_sm_value(void);

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
  runtime_data.is_sun_enabled = false;
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

    LL_RTC_WaitForSynchro(RTC);
    cfg_data.hms_time[H_POS] = (uint8_t)__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
    cfg_data.hms_time[M_POS] = (uint8_t)__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));

    sun_pwr_on();

    _handle_setup(true);
  }
  else
  {
    // TODO save cfg settings to flash?
    show_time_and_alarm_active();
    show_alarm_state();
    sun_pwr_off();

    LL_RTC_EnterInitMode(RTC);
    LL_RTC_TIME_SetHour(RTC, __LL_RTC_CONVERT_BIN2BCD(cfg_data.hms_time[H_POS]));
    LL_RTC_TIME_SetMinute(RTC, __LL_RTC_CONVERT_BIN2BCD(cfg_data.hms_time[M_POS]));
    LL_RTC_ExitInitMode(RTC);

    _set_alarm_start_time();
  }
}

bool is_setup_mode(void)
{
  return runtime_data.is_setup_mode;
}

void set_sun_enabled(bool is_enabled)
{
  runtime_data.is_sun_enabled = is_enabled;
}

bool is_sun_enabled(void)
{
  return runtime_data.is_sun_enabled;
}

void toggle_sun_state(void)
{
  if (is_sun_enabled())
  {
    sun_pwr_off();
    set_sun_enabled(false);
  }
  else
  {
    sun_set_intensity(cfg_data.sun_manual_intensity);
    sun_pwr_on();
    set_sun_enabled(true);
  }
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

  s = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
  if (is_alarm_active())
  {
    // alarm is already in progress
    // only handle alarm per minute basis
    if (s == 0)
    {
      h = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
      if (h == cfg_data.alarm_time[H_POS])
      {
        m = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
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
      h = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
      if (h == runtime_data.alarm_start_time[H_POS])
      {
        m = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
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

  case SETUP_MUSIC:
    _handle_setup_music(force_refresh);
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

void _handle_setup_music(bool force_refresh)
{
  show_setup_item(sm_area, "<not yet>");
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
      count *= 60;
    }
    _manipulate_time(&cfg_data.hms_time[H_POS], &cfg_data.hms_time[M_POS], count);

    time_to_str(time_str, cfg_data.hms_time[H_POS], cfg_data.hms_time[M_POS], -1);
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
