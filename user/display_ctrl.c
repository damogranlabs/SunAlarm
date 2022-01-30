/*
 * display.c
 *
 *  Created on: 27 nov 2021
 *      Author: domen
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "lcd.h"

#include "main.h"
#include "rtc.h"

#include "logic.h"
#include "display_ctrl.h"

#define LCD_X_SIZE 15

// time and alarm active status position
#define T_A_Y 0
#define A_LCD_POSITION 0 // LCD internal storage position index

// alarm time info position
#define A_SETTINGS_Y 1
#define A_SETTINGS_X 0
#define A_TIME_XPOS 6
#define A_ON_TEXT "Kava ob:"
#define A_OFF_TEXT "Brez alarma."

// settings menu string position
#define M_AREA_Y 0
#define M_AREA_X 0
#define M_VALUE_Y 1
#define M_VALUE_X 0

#define M_AREA_WAKEUP_TIME "Vzhod"
#define M_AREA_SUN_INTENSITY "Sonce"
#define M_AREA_SUN_MANUAL_INTENSITY "Nocna lucka"
#define M_AREA_TIME "Ura"

void _print_time(uint8_t y, uint8_t x, uint8_t *h, uint8_t *m, uint8_t *s);

extern configuration_t cfg_data;

static uint32_t lcd_off_timestamp;

// https://www.makerguides.com/character-lcd-arduino-tutorial/
static uint8_t alarm_active_icon[] = {0b00100,
                                      0b01110,
                                      0b01110,
                                      0b01110,
                                      0b11111,
                                      0b00000,
                                      0b00100,
                                      0b00000};

void create_alarm_status_icon(void)
{
  lcd_create_char(A_LCD_POSITION, alarm_active_icon);
}

void show_default(void)
{
  // this function show displays state on setup menu exit
  lcd_clear();

  show_time();
  show_alarm_active();
  show_alarm_state();
}

void show_time_and_alarm_active(void)
{
  show_alarm_active();
  show_time();
}

void show_alarm_active(void)
{
  // show alarm active status on the far left, time on the far right
  lcd_clear_area(T_A_Y, 0, 1);
  if (is_alarm_active())
  {
    lcd_put_char(T_A_Y, 0, A_LCD_POSITION);
  }
}

void show_time(void)
{
  // show alarm active status on the far left, time on the far right
  uint8_t h, m, s = 0;

  get_current_time(&h, &m, &s);

  lcd_clear_area(T_A_Y, LCD_X_SIZE - TIME_HMS_STR_SIZE, LCD_X_SIZE);
  _print_time(T_A_Y, LCD_X_SIZE - 8 + 1, &h, &m, &s); // 8 == 'HH:MM:SS'
}

void show_alarm_state()
{
  lcd_clear_area(A_SETTINGS_Y, A_SETTINGS_X, LCD_X_SIZE);
  if (cfg_data.is_alarm_enabled)
  {
    lcd_print_str(A_SETTINGS_Y, A_SETTINGS_X, A_ON_TEXT);
    _print_time(A_SETTINGS_Y,
                LCD_X_SIZE - TIME_HM_STR_SIZE + 1,
                &cfg_data.alarm_time[H_POS],
                &cfg_data.alarm_time[M_POS], NULL);
  }
  else
  {
    lcd_print_str(A_SETTINGS_Y, A_SETTINGS_X, A_OFF_TEXT);
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

void handle_lcd_backlight(void)
{
  if (lcd_off_timestamp != 0)
  {
    if (GetTick() > lcd_off_timestamp)
    {
      ctrl_lcd_backlight(false, false);
      lcd_off_timestamp = 0;
    }
  }
}

void ctrl_lcd_backlight(bool is_enabled, bool auto_backlight)
{
  // PNP transistor
  if (is_enabled)
  {
    LL_GPIO_ResetOutputPin(LCD_BACKLIGHT_Port, LCD_BACKLIGHT_Pin);
    if (auto_backlight)
    {
      lcd_off_timestamp = GetTick() + LCD_BACKLIGHT_OFF_DELAY_MS;
    }
    else
    {
      lcd_off_timestamp = 0;
    }
  }
  else
  {
    LL_GPIO_SetOutputPin(LCD_BACKLIGHT_Port, LCD_BACKLIGHT_Pin);
    lcd_off_timestamp = 0;
  }
}

void time_to_str(char *time_str, uint8_t *h, uint8_t *m, uint8_t *s)
{
  // helper func
  // print time on lcd in HH:MM:SS format, at given position.
  // Omit seconds if s==NULL
  if (s == NULL)
  {
    sprintf(time_str, "%2d:%02d", *h, *m);
  }
  else
  {
    sprintf(time_str, "%2d:%02d:%02d", *h, *m, *s);
  }
}

void _print_time(uint8_t y, uint8_t x, uint8_t *h, uint8_t *m, uint8_t *s)
{
  // helper func
  // print time on lcd in HH:MM:SS format, at given position.
  // Omit seconds if s==NULL
  char time_str[TIME_HMS_STR_SIZE + 1];
  if (s == NULL)
  {
    time_to_str(time_str, h, m, NULL);
  }
  else
  {
    time_to_str(time_str, h, m, s);
  }
  lcd_print_str(y, x, time_str);
}