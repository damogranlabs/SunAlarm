/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 *
 * SunAlarm
 * @date    Jan-2022
 * @author  Domen Jurkovic
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs/SunAlarm
 *
 * ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "buttons.h"
#include "rot_enc.h"
#include "lcd.h"

#include "logic.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern volatile bool rtc_event;
extern rot_enc_data_t encoder;
extern button_t buttons[NUM_OF_BUTTONS];

volatile uint32_t systick_counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void handle_systick_overflow(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  LL_SYSTICK_EnableIT();

  btn_register(buttons, B_SETUP_Port, B_SETUP_Pin, BTN_MODE_LONGPRESS);
  btn_register(buttons, B_LA_CTRL_Port, B_LA_CTRL_Pin, BTN_MODE_LONGPRESS);

  rot_enc_init(&encoder, ENC_A_GPIO_Port, ENC_A_Pin, ENC_B_GPIO_Port, ENC_B_Pin);
  rot_enc_set_direction(&encoder, ROT_ENC_INC_CCW);

  lcd_init(2, 16);
  lcd_clear();
  create_status_icons();
  ctrl_lcd_backlight(true, true);

  set_defaults();
  sun_init();
  show_alarm_state();

  /*
  // just for debugging
  extern configuration_t cfg_data;
  set_new_time(6, 18, 54);
  cfg_data.wakeup_time_min = 1;
  cfg_data.wakeup_timeout_min = 1;
  cfg_data.alarm_time[0] = 6;
  cfg_data.alarm_time[1] = 20;
  set_alarm_state(true);
  */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (rtc_event)
    {
      if (!is_setup_mode())
      {
        show_time_and_alarm_active();

        handle_systick_overflow();

        if (is_alarm_enabled())
        {
          handle_alarm();
        }
      }

      rtc_event = false;
    }
    if (is_alarm_enabled())
    {
      if (is_alarm_active())
      {
        handle_alarm_intensity(false);
      }
      else
      {
        handle_alarm_timeout();
      }
    }
    btn_handle(buttons);

    handle_interactions();

    handle_lcd_backlight();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  }
  LL_RCC_HSI_Enable();

  /* Wait till HSI is ready */
  while (LL_RCC_HSI_IsReady() != 1)
  {
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_LSI_Enable();

  /* Wait till LSI is ready */
  while (LL_RCC_LSI_IsReady() != 1)
  {
  }
  LL_PWR_EnableBkUpAccess();
  if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSI)
  {
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
  }
  LL_RCC_EnableRTC();
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

  /* Wait till System clock is ready */
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  }
  LL_Init1msTick(8000000);
  LL_SetSystemCoreClock(8000000);
}

/* USER CODE BEGIN 4 */
uint32_t GetTick(void)
{
  return systick_counter;
}

void handle_systick_overflow(void)
{
  // To avoid buttons deadlock every 49 days (systick 32-bit variable overflow),
  // reset systick variable and other dependant variables on a pre-defined time
  uint8_t h, m, s;

  // Skip procedure if alarm is active/setup mode
  // (hopefully no one has their alarm/setup mode regularly set at such crazy times
  if (!is_alarm_active())
  {
    if (!is_setup_mode())
    {
      get_current_time(&h, &m, &s);
      if ((h == SYSTICK_RESET_HOUR) && (m == SYSTICK_RESET_MIN) && (s == SYSTICK_RESET_SEC))
      {
        reset_lcd_backlight_time_on_systick_overflow();
        btn_reset_timestamps(buttons);
        systick_counter = 0;
      }
    }
  }
  // else: just try next day. once in 49 days will probably go through.
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
