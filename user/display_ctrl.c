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
// LCD internal storage position index
#define A_ACTIVE_LCD_POS 0
#define A_TIME_SETUP_LCD_POS 1

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
extern runtime_data_t runtime_data;

static uint32_t lcd_off_timestamp;

// https://maxpromer.github.io/LCD-Character-Creator/
static uint8_t alarm_active_icon[] = {0b00100,
                                      0b01110,
                                      0b01110,
                                      0b01110,
                                      0b11111,
                                      0b00000,
                                      0b00100,
                                      0b00000};
static uint8_t alarm_time_setup_icon[] = {0b00000,
                                          0b00000,
                                          0b00100,
                                          0b00110,
                                          0b11111,
                                          0b00110,
                                          0b00100,
                                          0b00000};

void create_status_icons(void)
{
  lcd_create_char(A_ACTIVE_LCD_POS, alarm_active_icon);
  lcd_create_char(A_TIME_SETUP_LCD_POS, alarm_time_setup_icon);
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
    lcd_put_char(T_A_Y, 0, A_ACTIVE_LCD_POS);
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
    if (runtime_data.is_alarm_time_setup_mode)
    {
      lcd_put_char(A_SETTINGS_Y, LCD_X_SIZE - TIME_HM_STR_SIZE, A_TIME_SETUP_LCD_POS);
    }
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

void fix_lcd_backlight_time_on_systick_overflow(void)
{
  if (lcd_off_timestamp != 0)
  {
    lcd_off_timestamp = lcd_off_timestamp - GetTick();
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