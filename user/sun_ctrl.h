/*
 * sun_ctrl.h
 *
 *  Created on: 25 Apr 2021
 *      Author: domen
 */

#ifndef SUN_CTRL_H_
#define SUN_CTRL_H_

#include <stdint.h>

// TIM1 CH4, PA11, PIR2

#define SUN_TIM TIM1
#define SUN_TIM_CH LL_TIM_CHANNEL_CH4

#define SUN_INTENSITY_MIN 0
#define SUN_INTENSITY_MAX 255

void sun_init(void);

void sun_pwr_on(void);
void sun_pwr_off(void);
void sun_pwr_toggle(void);

void sun_set_intensity(uint8_t intensity);
void sun_set_intensity_precise(uint32_t intensity);

uint32_t get_sun_intensity_resolution(void);

#endif /* SUN_CTRL_H_ */
