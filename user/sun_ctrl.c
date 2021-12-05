/*
 * sun_ctrl.c
 *
 *  Created on: 25 Apr 2021
 *      Author: domen
 */

#include <stdbool.h>

#include "stm32f0xx_ll_tim.h"

#include "sun_ctrl.h"

// TIM1 CH4, PA11, PIR2

#define SUN_TIM TIM1
#define SUN_TIM_CH LL_TIM_CHANNEL_CH4

void sun_init(void)
{
  LL_TIM_EnableAllOutputs(SUN_TIM);
}

void _sun_pwr_ctrl(bool state)
{
  if (state)
  {
    LL_TIM_CC_EnableChannel(SUN_TIM, SUN_TIM_CH);
    LL_TIM_EnableCounter(SUN_TIM);
  }
  else
  {
    LL_TIM_CC_DisableChannel(SUN_TIM, SUN_TIM_CH);
    LL_TIM_DisableCounter(SUN_TIM);
  }
}

void sun_pwr_on(void)
{
  _sun_pwr_ctrl(true);
}

void sun_pwr_off(void)
{
  _sun_pwr_ctrl(false);
}

void sun_set_intensity(uint8_t intensity)
{
  assert_param(intensity <= 255);
  assert_param(intensity >= 0);

  // percent to value conversion
  uint32_t compare_val = (LL_TIM_GetAutoReload(SUN_TIM) / SUN_INTENSITY_MAX) * intensity;
  LL_TIM_OC_SetCompareCH4(SUN_TIM, compare_val);
}
