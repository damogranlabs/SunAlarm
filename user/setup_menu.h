/*
 * setup_menu.h
 *
 *  Created on: Feb 14, 2021
 *      Author: domen
 */

#ifndef SRC_SETUP_MENU_H_
#define SRC_SETUP_MENU_H_

#include <stdbool.h>

#define DEFAULT_WAKEUP_TIME_MIN 3
#define DEFAULT_SUN_INTENSITY_MIN 10
#define DEFAULT_SUN_INTENSITY_MAX SUN_INTENSITY_MAX
#define DEFAULT_SUN_INTENSITY SUN_INTENSITY_MAX / 2
#define DEFAULT_ALARM_TIME_H 6
#define DEFAULT_ALARM_TIME_M 30

#define H_POS 0
#define M_POS 1
#define S_POS 2

typedef enum
{
  SETUP_WAKEUP_TIME,
  SETUP_SUN_MIN_INTENSITY,
  SETUP_SUN_MAX_INTENSITY,
  SETUP_SUN_DEFAULT_INTENSITY,
  SETUP_MUSIC,
  SETUP_TIME_H,
  SETUP_TIME_M,
  SETUP_DONE
} sm_area_t;

typedef struct configuration_t
{
  bool is_alarm_enabled;
  uint8_t hms_time[3];   //  HMS
  uint8_t alarm_time[2]; //  HM
  uint8_t wakeup_time_min;
  uint8_t sun_intensity_min;
  uint8_t sun_intensity_max;
  uint8_t sun_manual_intensity;

  // TODO setup music
} configuration_t;

typedef struct runtime_mode_t
{
  bool is_setup_mode;
  bool is_sun_enabled;
  bool is_alarm_active;
} runtime_mode_t;

void set_defaults(void);
void set_alarm_state(bool is_enabled);
bool is_alarm_enabled(void);
bool is_alarm_active(void);

void set_setup_mode(bool is_enabled);
bool is_setup_mode(void);

bool is_sun_enabled(void);
void set_sun_enabled(bool is_enabled);
void toggle_sun_state(void);

void handle_interactions(void);

void update_settings(void);

#endif /* SRC_SETUP_MENU_H_ */
