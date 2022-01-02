/**
 ******************************************************************************
 * File Name          : lcd_user.h
 * Description        : This file provides code for the configuration
 *                      of HD44780 based LCD
 * @date    13-Feb-2021
 * @author  Domen Jurkovic, Damogran Labs
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs
 * @version v1.4
*/

#ifndef __LCD_USER_H
#define __LCD_USER_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "main.h"

// define GPIO port/pin object type
#define LCD_GPIO_PORT_TYPE GPIO_TypeDef
#define LCD_GPIO_PIN_TYPE uint32_t

// pins defined by CubeMX .ioc in main.h

//#define LCD_GO_TO_NEW_LINE_IF_STRING_TOO_LONG	// uncomment if strings larger than screen size should break and continue on new line.
#define LCD_WINDOW_PRINT_DELAY_MS 800 // delay between static view and window scrolling (used in LCD_PrintStringWindow();)

void lcd_delay_us(uint32_t uSec);
void lcd_delay_ms(uint32_t mSec);

void lcd_init_pins(void);
void lcd_write_pin(LCD_GPIO_PORT_TYPE *port, LCD_GPIO_PIN_TYPE pin, bool state);

#endif /* __LCD_H */
