#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "stm32f0xx_ll_utils.h"
#include "rtc.h"

#include "lcd.h"
#include "buttons.h"
#include "rot_enc.h"

#include "logic.h"
#include "flash.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"

void _set_alarm_defaults(void);
void _set_alarm_start_time(void);
void _handle_alarm_time(void);
void _handle_current_intensity(void);

void _handle_setup(bool force_refresh);
void _handle_setup_alarm_time(void);
void _handle_setup_time(bool force_refresh, bool h_or_m);
void _handle_setup_sun_intensity(bool force_refresh);
void _handle_setup_sun_manual_intensity(bool force_refresh);
void _handle_setup_wakeup_time(bool force_refresh);
void _handle_setup_wakeup_timeout(bool force_refresh);

uint32_t _get_alarm_sun_intensity(void);

void _manipulate_time(uint8_t *h_ptr, uint8_t *m_ptr, int16_t change_min);

sm_area_t sm_area;
configuration_t cfg_data;
runtime_data_t runtime_data;
rot_enc_data_t encoder;
button_t buttons[NUM_OF_BUTTONS];

volatile bool rtc_event = false;

void _set_alarm_defaults(void)
{
  if (is_cfg_data_in_flash())
  {
    flash_read();
    set_new_time(cfg_data.time[H_POS], cfg_data.time[M_POS], 0);
  }
  else
  {
    cfg_data.is_alarm_enabled = false;
    cfg_data.alarm_time[H_POS] = DEFAULT_ALARM_TIME_H;
    cfg_data.alarm_time[M_POS] = DEFAULT_ALARM_TIME_M;
    cfg_data.wakeup_time_min = DEFAULT_WAKEUP_TIME_MIN;
    cfg_data.wakeup_timeout_min = DEFAULT_WAKEUP_TIMEOUT_MIN;
    cfg_data.sun_intensity_max = DEFAULT_SUN_INTENSITY_MAX;
    cfg_data.sun_manual_intensity = DEFAULT_SUN_INTENSITY;
  }
  cfg_data.intensity_resolution = get_sun_intensity_resolution();
  cfg_data.sun_intensity_max_precise = get_sun_intensity_value(cfg_data.sun_intensity_max);
}

void set_defaults(void)
{
  _set_alarm_defaults();

  runtime_data.is_setup_mode = false;
  runtime_data.is_alarm_time_setup_mode = false;
  runtime_data.is_alarm_active = false;
  runtime_data.alarm_start_timestamp = 0;
  runtime_data.alarm_timeout_timestamp = 0;
  runtime_data.last_alarm_intensity_timestamp = 0;
  runtime_data.setup_mode_end_timestamp = 0;

  _set_alarm_start_time();
}

void set_setup_mode(bool is_enabled)
{
  runtime_data.is_setup_mode = is_enabled;
  if (is_enabled)
  {
    sm_area = SETUP_WAKEUP_TIME;
    runtime_data.setup_mode_end_timestamp = GetTick() + (SETUP_MODE_TIMEOUT_SEC * 1000);

    get_current_time(&cfg_data.time[H_POS], &cfg_data.time[M_POS], NULL);

    sun_set_intensity(0);
    sun_pwr_on();

    _handle_setup(true);
  }
  else
  {
    set_new_time(cfg_data.time[H_POS], cfg_data.time[M_POS], 0);
    flash_write();
    sun_pwr_off();

    _set_alarm_start_time();

    show_default();
  }
}

bool is_setup_mode(void)
{
  return runtime_data.is_setup_mode;
}

void set_alarm_time_setup_mode(bool is_enabled)
{
  runtime_data.is_alarm_time_setup_mode = is_enabled;
  if (is_enabled)
  {
    runtime_data.setup_mode_end_timestamp = GetTick() + (SETUP_MODE_TIMEOUT_SEC * 1000);
  }
  show_alarm_state();
}

bool is_alarm_time_setup_mode(void)
{
  return runtime_data.is_alarm_time_setup_mode;
}

void update_settings(void)
{
  sm_area++;
  if (sm_area >= SETUP_DONE)
  {
    sm_area = SETUP_WAKEUP_TIME;
  }

  _handle_setup(true);
}

void handle_interactions(void)
{
  /* Runtime interactions:
  Light btn        |   Setup btn     |   Encoder changes
  -----------------+-----------------+------------------
          /        |       /         | If light is enabled: vary light intensity.
                   |                 | If alarm & display is enabled, vary alarm time.
                   |                 | In alarm time setup mode: vary setup item value.
                   |                 | Else: ignore.
  -----------------+-----------------+-----------------------------------------------
       press       |       /         |         /     -> light button toggle display and then light at manual intensity.
                   |                 |               -> In setup mode: ignore.
                   |                 |               -> In alarm time setup mode: disable alarm time setup mode.
                   |                 |               -> In alarm timeout period: disable alarm timeout period.
  -----------------+-----------------+-----------------------------------------------
      long press   |       /         |         /     -> if not in setup mode: enable alarm time setup mode.
  -----------------+-----------------+-----------------------------------------------
         /         |     press       |         /     -> If alarm is active: deactivate alarm.
                   |                 |                  Else: toggle alarm state.
                   |                 |               -> In setup mode: move to the next menu item.
  -----------------+-----------------+-----------------------------------------------
         /         |   long press    |         /     -> Enter/exit alarm setup mode.
  -----------------+-----------------+-----------------------------------------------
  */
  bool setup_mode = is_setup_mode();
  bool alarm_time_setup_mode = is_alarm_time_setup_mode();
  bool menu_timeout = false;

  if (GetTick() > runtime_data.setup_mode_end_timestamp)
  {
    menu_timeout = true;
  }

  if (setup_mode)
  {
    if (menu_timeout)
    {
      set_setup_mode(false);
      ctrl_lcd_backlight(true, true);
    }
    else
    {
      _handle_setup(false);
    }
  }
  else if (alarm_time_setup_mode)
  {
    if (menu_timeout)
    {
      set_alarm_time_setup_mode(false);
      ctrl_lcd_backlight(true, true);
    }
    else
    {
      _handle_alarm_time();
    }
  }
  else
  {
    if (is_sun_enabled())
    {
      _handle_current_intensity();
    }
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
          // do not power off. just let the user or timeout turn the sun/alarm off.
          runtime_data.alarm_timeout_timestamp = GetTick() + (cfg_data.wakeup_timeout_min * 60 * 1000);

          return;
        }
      }
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
          ctrl_lcd_backlight(true, false); // keep the LCD backlight ON
          set_alarm_active(true);
          handle_alarm_intensity(true);
          sun_pwr_on();
        }
      }
    }
  }
}

void handle_alarm_intensity(bool restart)
{
  if (restart)
  {
    cfg_data.sun_intensity_max_precise = get_sun_intensity_value(cfg_data.sun_intensity_max);
    cfg_data.wakeup_time_ms = ((uint32_t)cfg_data.wakeup_time_min) * 60 * 1000;
    runtime_data.alarm_start_timestamp = GetTick();
    sun_set_intensity_precise(_get_alarm_sun_intensity());
    runtime_data.last_alarm_intensity_timestamp = GetTick();
  }

  if (GetTick() > runtime_data.last_alarm_intensity_timestamp)
  {
    sun_set_intensity_precise(_get_alarm_sun_intensity());
    runtime_data.last_alarm_intensity_timestamp = GetTick();
  }
}

void handle_alarm_timeout(void)
{
  if (is_sun_enabled())
  {
    if (is_alarm_timeout_pending())
    {
      if (GetTick() > runtime_data.alarm_timeout_timestamp)
      {
        sun_pwr_off();
        runtime_data.alarm_timeout_timestamp = 0;
      }
    }
  }
}

uint32_t _get_alarm_sun_intensity(void)
{
  // y = kx + n
  // intensity = (max/dur)*time + intensity_min
  //
  // max = intensity range (sun max user settings) scaled to TIM PWM range.
  // dur = wakeup length in minutes, scaled to msec
  // time is time that intensity is searched for in msec
  // note: final function is written just a bit different to avoid division errors
  //       because of rounding (integers instead of floats)
  uint32_t time_ms = GetTick() - runtime_data.alarm_start_timestamp;
  uint32_t intensity = (uint32_t)((((uint64_t)cfg_data.sun_intensity_max_precise * (uint64_t)time_ms) / (uint64_t)cfg_data.wakeup_time_ms));
  if (intensity > cfg_data.sun_intensity_max_precise)
  {
    return cfg_data.sun_intensity_max_precise;
  }
  else
  {
    return intensity;
  }
}

void _handle_setup(bool force_refresh)
{
  switch (sm_area)
  {
  case SETUP_WAKEUP_TIME:
    _handle_setup_wakeup_time(force_refresh);
    break;

  case SETUP_WAKEUP_TIMEOUT:
    _handle_setup_wakeup_time(force_refresh);
    break;

  case SETUP_SUN_MAX_INTENSITY:
    _handle_setup_sun_intensity(force_refresh);
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

  show_alarm_active();
}

bool is_alarm_timeout_pending(void)
{
  if (runtime_data.alarm_timeout_timestamp == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

void reset_alarm_timeout_timestamp(void)
{
  runtime_data.alarm_timeout_timestamp = 0;
}

// modify alarm settings in the runtime
void _handle_alarm_time(void)
{
  int16_t count = rot_enc_get_count(&encoder);
  // alarm is set in minutes
  if (count != 0)
  {
    count *= ALARM_TIME_MOD_FACTOR;
    _manipulate_time(&cfg_data.alarm_time[H_POS], &cfg_data.alarm_time[M_POS], count);
    _set_alarm_start_time();

    show_alarm_state();
  }
}
// modify current intensity with encoder, but don't toggle default nightlight intensity value
void _handle_current_intensity(void)
{
  int16_t intensity;
  int16_t count = rot_enc_get_count(&encoder);
  if (count != 0)
  {
    intensity = (int16_t)sun_get_intensity();
    count *= SUN_INTENSITY_MOD_FACTOR;

    if ((intensity + count) <= 0)
    {
      sun_set_intensity(0);
    }
    else if ((intensity + count) > SUN_INTENSITY_MAX)
    {
      sun_set_intensity(SUN_INTENSITY_MAX);
    }
    else
    {
      sun_set_intensity((uint8_t)(intensity + count));
    }
  }
}

void _handle_setup_wakeup_time(bool force_refresh)
{
  char time_str[4]; // uint8_t + null char

  int32_t count = rot_enc_get_count(&encoder);
  if ((count != 0) || (force_refresh == true))
  {
    count *= WAKEUP_TIME_MOD_FACTOR;
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

void _handle_setup_wakeup_timeout(bool force_refresh)
{
  char time_str[4]; // uint8_t + null char

  int32_t count = rot_enc_get_count(&encoder);
  if ((count != 0) || (force_refresh == true))
  {
    if ((cfg_data.wakeup_timeout_min + count) > 255)
    {
      cfg_data.wakeup_timeout_min = 255;
    }
    else if ((cfg_data.wakeup_timeout_min + count) < 0)
    {
      cfg_data.wakeup_timeout_min = 1;
    }
    else
    {
      cfg_data.wakeup_timeout_min += count;
    }
    sprintf(time_str, "%d", cfg_data.wakeup_timeout_min);
    show_setup_item(sm_area, &time_str[0]);
  }
}

void _handle_setup_time(bool force_refresh, bool change_hours)
{
  // change_hours: if True, rotary encoder is setting hours, else minutes
  char time_str[TIME_HM_STR_SIZE];
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

    time_to_str(time_str, &cfg_data.time[H_POS], &cfg_data.time[M_POS], NULL);
    show_setup_item(sm_area, time_str);
  }
}

void _handle_setup_sun_intensity(bool force_refresh)
{
  // change_min_intensity: if True, rotary encoder is setting minimum intensity,
  //  else max intensity
  char time_str[4]; // 0 - SUN_INTENSITY_MAX
  int8_t count = rot_enc_get_count(&encoder);

  if ((count != 0) || (force_refresh == true))
  {
    if ((cfg_data.sun_intensity_max + count) > SUN_INTENSITY_MAX)
    {
      cfg_data.sun_intensity_max = SUN_INTENSITY_MAX;
    }
    else if ((cfg_data.sun_intensity_max + count) <= 0)
    {
      cfg_data.sun_intensity_max = 1;
    }
    else
    {
      cfg_data.sun_intensity_max += count;
    }
    sun_set_intensity(cfg_data.sun_intensity_max);
    sprintf(time_str, "%d", cfg_data.sun_intensity_max);
    show_setup_item(sm_area, time_str);
  }
}

void _handle_setup_sun_manual_intensity(bool force_refresh)
{
  char val_str[3]; // 0 - SUN_INTENSITY_MAX
  int16_t count = rot_enc_get_count(&encoder);

  if ((count != 0) || (force_refresh == true))
  {
    count += (int16_t)cfg_data.sun_manual_intensity;
    if (count > SUN_INTENSITY_MAX)
    {
      cfg_data.sun_manual_intensity = SUN_INTENSITY_MAX;
    }
    else if (count <= 0)
    {
      cfg_data.sun_manual_intensity = 0;
    }
    else
    {
      cfg_data.sun_manual_intensity = (uint8_t)count;
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
