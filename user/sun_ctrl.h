#ifndef SUN_CTRL_H_
#define SUN_CTRL_H_

#include <stdint.h>

#include "stm32f0xx_ll_tim.h"

// TIM1 CH4, PA11, PIR2

#define SUN_TIM TIM1
#define SUN_TIM_CH LL_TIM_CHANNEL_CH4

#define SUN_INTENSITY_MAX 100

void sun_init(void);

void sun_pwr_on(void);
void sun_pwr_off(void);
void sun_pwr_on_manual(void);

uint8_t sun_get_intensity(void);
void sun_set_intensity(uint8_t intensity);
void sun_set_intensity_precise(uint32_t intensity);

uint32_t get_sun_intensity_value(uint8_t user_intensity);

static inline bool is_sun_enabled(void)
{
    return (bool)LL_TIM_IsEnabledCounter(SUN_TIM);
}

static inline uint32_t get_sun_intensity_resolution(void)
{
    return LL_TIM_GetAutoReload(SUN_TIM) + 1; // +1: from 0 up to ARR
}

#endif /* SUN_CTRL_H_ */
