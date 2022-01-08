#ifndef __LOGIC_H_
#define __LOGIC_H_

#include <stdbool.h>

#define DEFAULT_WAKEUP_TIME_MIN 30
#define DEFAULT_SUN_INTENSITY_MIN 0
#define DEFAULT_SUN_INTENSITY_MAX SUN_INTENSITY_MAX
#define DEFAULT_SUN_INTENSITY SUN_INTENSITY_MAX / 2
#define DEFAULT_ALARM_TIME_H 6
#define DEFAULT_ALARM_TIME_M 30

#define H_POS 0
#define M_POS 1

typedef enum
{
  SETUP_WAKEUP_TIME,
  SETUP_SUN_MIN_INTENSITY,
  SETUP_SUN_MAX_INTENSITY,
  SETUP_SUN_DEFAULT_INTENSITY,
  SETUP_TIME_H,
  SETUP_TIME_M,
  SETUP_DONE
} sm_area_t;

typedef struct configuration_t
{
  bool is_alarm_enabled;
  uint8_t time[2];       //  HM
  uint8_t alarm_time[2]; //  HM
  uint8_t wakeup_time_min;
  uint8_t sun_intensity_min;
  uint8_t sun_intensity_max;
  uint8_t sun_manual_intensity;

  // used for setting alarm sun intensity (avoid re-calculating these static values all the time)
  uint32_t sun_intensity_min_precise;
  uint32_t sun_intensity_max_precise;
  uint32_t sun_intensity_diff_precise;
  uint32_t wakeup_time_ms;
} configuration_t;

typedef struct runtime_data_t
{
  bool is_setup_mode;
  bool is_alarm_active;
  uint8_t alarm_start_time[2];             // HM
  uint32_t alarm_start_timestamp;          // msec
  uint32_t last_alarm_intensity_timestamp; // msec

} runtime_data_t;

void set_defaults(void);
void set_alarm_state(bool is_enabled);
bool is_alarm_enabled(void);
bool is_alarm_active(void);
void set_alarm_active(bool is_active);

void set_setup_mode(bool is_enabled);
bool is_setup_mode(void);

void handle_interactions(void);
void handle_alarm(void);
void handle_alarm_intensity(bool restart);

void update_settings(void);

#endif /* __LOGIC_H_ */
