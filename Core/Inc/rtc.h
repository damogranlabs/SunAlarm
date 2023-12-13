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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
// ATM, current PCB does not have external crystal for RTC. Internal LSI is used, which is really bad, but c'est a la vie.
// Set following RTC prescaller settings to get 1 second as precise as possible - measure second duration via SEC_TOGGLE_Pin.
// RTC_ASYNCHPRESCALER: max 0x7F (127)
// RTC_SYNCHPRESCALER: max 0x7FFF (32767)
//
// For the record: I have no idea how these prescalers affect the RTC clock. I couldn't figure out how to reliably
// predict the numbers, as measurements did not align with the reference manual and STM notes.
// https://www.st.com/resource/en/application_note/an3371-using-the-hardware-realtime-clock-rtc-in-stm32-f0-f2-f3-f4-and-l1-series-of-mcus-stmicroelectronics.pdf
#ifdef NEJCS_PCB
// Each second is about 40us ahead.
// That is about 3.45 second/per day, about 1.7 min/month (30 days)
#define RTC_ASYNCHPRESCALER 4
// #define RTC_SYNCHPRESCALER 8190 // 499.57
// #define RTC_SYNCHPRESCALER 8185 // 499.83-87
// #define RTC_SYNCHPRESCALER 8180 // 500.17
#define RTC_SYNCHPRESCALER 8182 // 500.04mHz // 500.0(1-3)mHz
  // #define RTC_SYNCHPRESCALER 8183 // 499.96 mHz
  // #define RTC_SYNCHPRESCALER 8184 // 499.91  mHz

#elif DOMENS_PCB
// Each second is about 20us behind.
// That is about 1.7 second/per day, about 51.8 sec/month (30 days)
#define RTC_ASYNCHPRESCALER 4
#define RTC_SYNCHPRESCALER 8022

#else
#error "PCB not selected: missing RTC_ASYNCHPRESCALER and RTC_SYNCHPRESCALER settings (defines)."
#endif

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
