/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
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
  RTC_InitStruct.AsynchPrescaler = 123; // 127; for 994ms, 124 for 1002
  RTC_InitStruct.SynchPrescaler = 329;  //  317; for 994ms, 327 for 1002
  LL_RTC_Init(RTC, &RTC_InitStruct);
  //LL_RTC_SetAsynchPrescaler(RTC, 124); //127
  //LL_RTC_SetSynchPrescaler(RTC, 319);  //311
  /* USER CODE BEGIN RTC_Init 2 */
  //NOTE: at this settings:
  //RTC_InitStruct.AsynchPrescaler = 123;
  //RTC_InitStruct.SynchPrescaler = 329;
  //... and this specific HW at room temperature, socend is about 30us short.
  // That is about 2.6 second/per day. -> each 23 days, you are 1 minute behind.
  LL_RTC_DisableWriteProtection(RTC);
  if (LL_RTC_WaitForSynchro(RTC) == ERROR)
  {
    Error_Handler();
  }

  /*
  while (LL_RTC_IsActiveFlag_RECALP(RTC) == 1)
  {
  }
  LL_RTC_CAL_SetMinus(RTC, 70);
  */

  // interrupt on 1 second
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_17;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  EXTI_InitStruct.LineCommand = ENABLE;
  if (LL_EXTI_Init(&EXTI_InitStruct) == ERROR)
  {
    Error_Handler();
  }

  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, 0);
  NVIC_EnableIRQ(RTC_IRQn);

  LL_RTC_ALMA_Disable(RTC);
  LL_RTC_ALMA_SetMask(RTC, LL_RTC_ALMA_MASK_ALL);
  LL_RTC_ClearFlag_ALRA(RTC);
  LL_RTC_EnableIT_ALRA(RTC);
  LL_RTC_ALMA_Enable(RTC);

  LL_RTC_EnableWriteProtection(RTC);
  /* USER CODE END RTC_Init 2 */
}

/* USER CODE BEGIN 1 */
void get_current_time(uint8_t *h, uint8_t *m, uint8_t *s)
{
  if (s != NULL)
  {
    *s = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
  }
  *m = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
  *h = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
  LL_RTC_ReadReg(RTC, DR);
}

void set_new_time(uint8_t h, uint8_t m, uint8_t s)
{
  LL_RTC_TimeTypeDef RTC_TimeStruct = {0};

  RTC_TimeStruct.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
  RTC_TimeStruct.Hours = h;
  RTC_TimeStruct.Minutes = m;
  RTC_TimeStruct.Seconds = s;

  if (LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_TimeStruct) == ERROR)
  {
    Error_Handler();
  }
}
/* USER CODE END 1 */
