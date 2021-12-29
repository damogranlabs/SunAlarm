#include <stdbool.h>

#include "stm32f0xx_ll_tim.h"

#include "sun_ctrl.h"

#include "logic.h"

extern configuration_t cfg_data;

void sun_init(void)
{
  LL_TIM_SetCounter(SUN_TIM, 0);
  sun_set_intensity(cfg_data.sun_manual_intensity);
  LL_TIM_EnableAllOutputs(SUN_TIM);
}

void _sun_pwr_ctrl(bool state)
{
  if (state)
  {
    LL_TIM_SetCounter(SUN_TIM, 0);
    LL_TIM_CC_EnableChannel(SUN_TIM, SUN_TIM_CH);
    LL_TIM_EnableCounter(SUN_TIM);
  }
  else
  {
    LL_TIM_CC_DisableChannel(SUN_TIM, SUN_TIM_CH);
    LL_TIM_DisableCounter(SUN_TIM);
    LL_TIM_SetCounter(SUN_TIM, 0);
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

void sun_pwr_on_manual(void)
{
  sun_set_intensity(cfg_data.sun_manual_intensity);
  sun_pwr_on();
}

bool is_sun_enabled(void)
{
  return (bool)LL_TIM_IsEnabledCounter(SUN_TIM);
}

uint32_t get_sun_intensity_resolution(void)
{
  return LL_TIM_GetAutoReload(SUN_TIM);
}

void sun_set_intensity(uint8_t intensity)
{
  assert_param(intensity <= SUN_INTENSITY_MAX);

  // percent to value conversion
  uint32_t compare_val = get_sun_intensity_resolution() / SUN_INTENSITY_MAX * intensity;

  LL_TIM_OC_SetCompareCH4(SUN_TIM, compare_val);
}

void sun_set_intensity_precise(uint32_t intensity)
{
  assert_param(intensity <= get_sun_intensity_resolution());

  LL_TIM_OC_SetCompareCH4(SUN_TIM, (uint32_t)intensity);
}
