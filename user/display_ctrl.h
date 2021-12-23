/*
 * display_ctrl.h
 *
 *  Created on: 27 Nov 2021
 *      Author: domen
 */

#ifndef DISPLAY_CTRL_H_
#define DISPLAY_CTRL_H_

#include <stdint.h>

#include "setup_menu.h"

#define TIME_STR_SIZE 8 // HH:MM:SS
#define LCD_BACKLIGHT_OFF_DELAY_MS 1000

void show_time_and_alarm_active(void);

void show_alarm_state(void);
void show_setup_item(sm_area_t sm_area, char *menu_str);

void handle_lcd_backlight(void);
void ctrl_lcd_backlight(bool is_enabled, bool auto_backlight);
bool is_lcd_backlight_enabled(void);

void time_to_str(char *time_str, uint8_t h, uint8_t m, int8_t s);

#endif /* DISPLAY_CTRL_H_ */
