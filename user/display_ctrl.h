/*
 * display_ctrl.h
 *
 *  Created on: 27 Nov 2021
 *      Author: domen
 */

#ifndef DISPLAY_CTRL_H_
#define DISPLAY_CTRL_H_

#include <stdint.h>

#include "stm32f0xx_ll_gpio.h"

#include "logic.h"

#define TIME_HMS_STR_SIZE 8 // HH:MM:SS
#define TIME_HM_STR_SIZE 5  // HH:MM
#define LCD_BACKLIGHT_OFF_DELAY_MS 4000

void create_status_icons(void);

void show_default(void);

void show_time_and_alarm_active(void);
void show_time(void);
void show_alarm_active(void);

void show_alarm_state(void);
void show_setup_item(sm_area_t sm_area, char *menu_str);

void handle_lcd_backlight(void);
void ctrl_lcd_backlight(bool is_enabled, bool auto_backlight);
void reset_lcd_backlight_time_on_systick_overflow(void);
static inline bool is_lcd_backlight_enabled(void)
{
    return !LL_GPIO_IsOutputPinSet(LCD_BACKLIGHT_Port, LCD_BACKLIGHT_Pin);
}

void time_to_str(char *time_str, uint8_t *h, uint8_t *m, uint8_t *s);

#endif /* DISPLAY_CTRL_H_ */
