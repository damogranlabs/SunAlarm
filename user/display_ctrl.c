/*
 * display.c
 *
 *  Created on: 27 nov 2021
 *      Author: domen
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f0xx_ll_rtc.h"

#include "lcd.h"

#include "main.h"
#include "setup_menu.h"
#include "display_ctrl.h"

#define LCD_X_SIZE 15

// time position
#define T_Y 0
#define T_X 1 // -1 = left aligned, 1 = right aligned.

// alarm current status (active) position
#define A_ACTIVE_Y 0
#define A_ACTIVE_X 0
// alarm time info position
#define A_SETTINGS_Y 1
#define A_SETTINGS_X 0
#define A_TIME_XPOS 6
#define A_ON_TEXT "Zbudi ob:"
#define A_OFF_TEXT "Brez alarma."

// settings menu string position
#define M_AREA_Y 0
#define M_AREA_X 0
#define M_VALUE_Y 1
#define M_VALUE_X 0

#define M_AREA_WAKEUP_TIME "Vzhod"
#define M_AREA_SUN_INTENSITY "Sonce"
#define M_AREA_SUN_MANUAL_INTENSITY "Luc"
#define M_AREA_MUSIC "MUSIC"
#define M_AREA_TIME "Ura"

void _print_time(uint8_t X, uint8_t y, uint8_t h, uint8_t m, int8_t s);

extern configuration_t cfg_data;

void show_time(void)
{
  uint8_t h, m, s = 0;

  h = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
  m = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
  s = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));

  lcd_clear_area(T_Y, 0, LCD_X_SIZE);
  if (T_X == -1)
  {
    // left-aligned
    _print_time(T_Y, 0, h, m, s);
  }
  else
  {
    // right-aligned
    // 8 == 'HH:MM:SS'
    _print_time(T_Y, LCD_X_SIZE - 8 + 1, h, m, s);
  }
}

void show_alarm_state()
{
  lcd_clear_area(A_SETTINGS_Y, A_SETTINGS_X, LCD_X_SIZE);
  if (cfg_data.is_alarm_enabled)
  {
    lcd_print_str(A_SETTINGS_Y, A_SETTINGS_X, A_ON_TEXT);
    _print_time(A_SETTINGS_Y, strlen(A_ON_TEXT) + 1, cfg_data.alarm_time[H_POS], cfg_data.alarm_time[M_POS], -1);
  }
  else
  {
    lcd_print_str(A_SETTINGS_Y, A_SETTINGS_X, A_OFF_TEXT);
  }
}

void show_alarm_active_state(bool is_active)
{
  if (is_active)
  {
    lcd_print_str(A_ACTIVE_Y, A_ACTIVE_X, "!");
  }
  else
  {
    lcd_print_str(A_ACTIVE_Y, A_ACTIVE_X, " ");
  }
}

void show_setup_item(sm_area_t sm_area, char *value_str)
{
  char setting_str[LCD_X_SIZE + 1];

  switch (sm_area)
  {
  case SETUP_WAKEUP_TIME:
    sprintf(setting_str, "@ %s (m):", M_AREA_WAKEUP_TIME);
    break;

  case SETUP_MUSIC:
    sprintf(setting_str, "@ %s:", M_AREA_MUSIC);
    break;

  case SETUP_SUN_MIN_INTENSITY:
    sprintf(setting_str, "@ %s (min):", M_AREA_SUN_INTENSITY);
    break;
  case SETUP_SUN_MAX_INTENSITY:
    sprintf(setting_str, "@ %s (max):", M_AREA_SUN_INTENSITY);
    break;

  case SETUP_SUN_DEFAULT_INTENSITY:
    sprintf(setting_str, "@ %s:", M_AREA_SUN_MANUAL_INTENSITY);
    break;

  case SETUP_TIME_H:
    sprintf(setting_str, "@ %s (h):", M_AREA_TIME);
    break;
  case SETUP_TIME_M:
    sprintf(setting_str, "@ %s (m):", M_AREA_TIME);
    break;

  default:
    Error_Handler();
  }

  lcd_clear();
  lcd_print_str(M_AREA_Y, M_AREA_X, setting_str);
  lcd_print_str(M_VALUE_Y, M_VALUE_X, value_str);
}

void time_to_str(char *time_str, uint8_t h, uint8_t m, int8_t s)
{
  // helper func
  // print time on lcd in HH:MM:SS format, at given position.
  // Omit seconds if s==-1
  if (s == -1)
  {
    sprintf(time_str, "%2d:%02d", (int)h, (int)m);
  }
  else
  {
    sprintf(time_str, "%2d:%02d:%02d", (int)h, (int)m, (int)s);
  }
}

void _print_time(uint8_t y, uint8_t x, uint8_t h, uint8_t m, int8_t s)
{
  // helper func
  // print time on lcd in HH:MM:SS format, at given position.
  // Omit seconds if s==-1
  char time_str[9];
  if (s == -1)
  {
    time_to_str(time_str, h, m, -1);
  }
  else
  {
    time_to_str(time_str, h, m, s);
  }
  lcd_print_str(y, x, time_str);
}