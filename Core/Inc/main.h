/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/

#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

  /* Private includes ----------------------------------------------------------*/
  /* USER CODE BEGIN Includes */

  /* USER CODE END Includes */

  /* Exported types ------------------------------------------------------------*/
  /* USER CODE BEGIN ET */

  /* USER CODE END ET */

  /* Exported constants --------------------------------------------------------*/
  /* USER CODE BEGIN EC */

  /* USER CODE END EC */

  /* Exported macro ------------------------------------------------------------*/
  /* USER CODE BEGIN EM */

  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

  /* USER CODE BEGIN EFP */
  uint32_t GetTick(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENC_A_Pin LL_GPIO_PIN_0
#define ENC_A_GPIO_Port GPIOF
#define ENC_A_EXTI_IRQn EXTI0_1_IRQn
#define ENC_B_Pin LL_GPIO_PIN_1
#define ENC_B_GPIO_Port GPIOF
#define B0_Pin LL_GPIO_PIN_0
#define B0_GPIO_Port GPIOA
#define OUT_OD_1_Pin LL_GPIO_PIN_1
#define OUT_OD_1_GPIO_Port GPIOA
#define LCD_RS_Pin LL_GPIO_PIN_8
#define LCD_RS_GPIO_Port GPIOA
#define PIR3_Pin LL_GPIO_PIN_9
#define PIR3_GPIO_Port GPIOA
#define LCD_E_Pin LL_GPIO_PIN_10
#define LCD_E_GPIO_Port GPIOA
#define PIR2_Pin LL_GPIO_PIN_11
#define PIR2_GPIO_Port GPIOA
#define PIR1_Pin LL_GPIO_PIN_12
#define PIR1_GPIO_Port GPIOA
#define LIM_SW2_Pin LL_GPIO_PIN_15
#define LIM_SW2_GPIO_Port GPIOA
#define LIM_SW1_Pin LL_GPIO_PIN_3
#define LIM_SW1_GPIO_Port GPIOB
#define LCD_D4_Pin LL_GPIO_PIN_4
#define LCD_D4_GPIO_Port GPIOB
#define LCD_D5_Pin LL_GPIO_PIN_5
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D6_Pin LL_GPIO_PIN_6
#define LCD_D6_GPIO_Port GPIOB
#define LCD_D7_Pin LL_GPIO_PIN_7
#define LCD_D7_GPIO_Port GPIOB
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0 ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority, \
                                                         4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1 ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority, \
                                                         3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2 ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority, \
                                                         2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3 ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority, \
                                                         1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4 ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority, \
                                                         0 bit  for subpriority */
#endif
  /* USER CODE BEGIN Private defines */

#define B_LA_CTRL_Pin LIM_SW1_Pin
#define B_LA_CTRL_Port LIM_SW1_GPIO_Port

#define B_SETUP_Pin LIM_SW2_Pin
#define B_SETUP_Port LIM_SW2_GPIO_Port

#define SUN_OUT_Pin PIR2_Pin
#define SUN_OUT_Port PIR2_GPIO_Port

#define LCD_BACKLIGHT_Pin PIR1_Pin
#define LCD_BACKLIGHT_Port PIR1_GPIO_Port

#define SEC_TOGGLE_Pin PIR3_Pin
#define SEC_TOGGLE_Port PIR3_GPIO_Port

// TIME that systick and related variables are reset (to avoid overflow issue)
#define SYSTICK_RESET_HOUR 3
#define SYSTICK_RESET_MIN 0
#define SYSTICK_RESET_SEC 0

  /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
