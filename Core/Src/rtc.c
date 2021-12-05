/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "main.h"

/* USER CODE END 0 */

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  LL_RTC_InitTypeDef RTC_InitStruct = {0};

  /* Peripheral clock enable */
  LL_RCC_EnableRTC();

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC and set the Time and Date
  */
  RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
  RTC_InitStruct.AsynchPrescaler = 127;
  RTC_InitStruct.SynchPrescaler = 255;
  LL_RTC_Init(RTC, &RTC_InitStruct);
  LL_RTC_SetAsynchPrescaler(RTC, 127);
  LL_RTC_SetSynchPrescaler(RTC, 255);
  /* USER CODE BEGIN RTC_Init 2 */
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableShadowRegBypass(RTC);

  // interrupt on 1 second
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_17;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  EXTI_InitStruct.LineCommand = ENABLE;
  LL_EXTI_Init(&EXTI_InitStruct);

  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, 0);
  NVIC_EnableIRQ(RTC_IRQn);

  LL_RTC_ALMA_Disable(RTC);
  LL_RTC_ALMA_SetMask(RTC, LL_RTC_ALMA_MASK_ALL);

  LL_RTC_ClearFlag_ALRA(RTC);
  LL_RTC_EnableIT_ALRA(RTC);
  LL_RTC_ALMA_Enable(RTC);
  /* USER CODE END RTC_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
