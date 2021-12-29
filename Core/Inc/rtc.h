/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// RTC prescaller settings to get 1 second as precise as possible
// Nejc board, first SunAlarm ever: this specific HW at room temperature, each second is about 30us short.
// That is about 2.6 second/per day. -> each 23 days, you are 1 minute behind.
#define RTC_ASYNCHPRESCALER 123
#define RTC_SYNCHPRESCALER 329

  // TODO modify defines according to your HW.

/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
  void get_current_time(uint8_t *h, uint8_t *m, uint8_t *s);
  void set_new_time(uint8_t h, uint8_t m, uint8_t s);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

