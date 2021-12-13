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
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"

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
#define OUT_OD_2_Pin LL_GPIO_PIN_2
#define OUT_OD_2_GPIO_Port GPIOA
#define MOTOR_2_Pin LL_GPIO_PIN_0
#define MOTOR_2_GPIO_Port GPIOB
#define MOTOR_1_Pin LL_GPIO_PIN_1
#define MOTOR_1_GPIO_Port GPIOB
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
/* USER CODE BEGIN Private defines */

#define B_LA_CTRL_Pin LIM_SW1_Pin
#define B_LA_CTRL_Port LIM_SW1_GPIO_Port

#define B_SETUP_Pin B0_Pin
#define B_SETUP_Port B0_GPIO_Port

#define SUN_OUT_Pin PIR2_Pin
#define SUN_OUT_Port PIR2_GPIO_Port

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
