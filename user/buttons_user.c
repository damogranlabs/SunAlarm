/*
 * Template library for handling button events.
 * @date    30-May-2020
 * @author  Domen Jurkovic
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs
 */
#include <stdint.h>

#include "buttons.h"
#include "buttons_user.h"

// add custom includes here to access required defines, cpu-specific LL functions, ...
#include "main.h"

#include "lcd.h"

#include "logic.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"

/**
 * @brief Get current system milliseconds (systick) value (usually started at system reset).
 * @retval Milliseconds value.
 * TODO: user must implement this function to return milliseconds (systick) value.
 */
uint32_t get_milliseconds(void)
{
  return GetTick();
}

/**
 * @brief Low level call to get actual pin state.
 * @param *btn_cfg Registered button configuration structure.
 * @retval BTN_PHY_ACTIVE on button press, BTN_PHY_IDLE otherwise.
 * TODO: user must implement this function to return button state.
 */
btn_phy_state_t get_button_pin_state(btn_cfg_t *btn_cfg)
{
  uint32_t pin_state = LL_GPIO_IsInputPinSet(btn_cfg->gpio_port, btn_cfg->gpio_pin);

  if (pin_state == 0)
  {
    return BTN_PHY_ACTIVE;
  }
  else
  {
    return BTN_PHY_IDLE;
  }
}

/**
 * @brief On press (short, repetitive) button callback
 * @param *btn_cfg Button configuration structure that triggered the event.
 * @retval None.
 * TODO: user can add actions on button events here.
 */
void on_button_press(btn_cfg_t *btn_cfg)
{
  if ((btn_cfg->gpio_port == B_SETUP_Port) && (btn_cfg->gpio_pin == B_SETUP_Pin))
  {
    if (is_alarm_active())
    {
      // use this button to cancel alarm
      set_alarm_active(false);
      ctrl_lcd_backlight(true, true);
    }
    else
    {
      // alarm not active, toggle runtime mode.
      if (is_setup_mode())
      {
        // in setup mode, move to next setup menu item
        update_settings();
      }
      else
      {
        // toggle alarm enabled/disabled state (state, not active status!)
        set_alarm_state(!is_alarm_enabled());
        ctrl_lcd_backlight(true, true);
      }
    }
  }
  else if ((btn_cfg->gpio_port == B_LA_CTRL_Port) && (btn_cfg->gpio_pin == B_LA_CTRL_Pin))
  {
    // use this button to only turn on LCD backlight (see time/alarm).
    // Press again in this time to power on sun (manual intensity)
    // Power off after next button press (LCD after timeout)
    if (!is_setup_mode())
    {
      if (is_lcd_backlight_enabled())
      {
        if (is_sun_enabled())
        {
          sun_pwr_off();
          ctrl_lcd_backlight(true, true);
        }
        else
        {
          sun_pwr_on_manual();
          ctrl_lcd_backlight(true, false);
        }
      }
      else
      {
        ctrl_lcd_backlight(true, true);
      }
    }
  }
}

/**
 * @brief On press (long) button callback
 * @param *btn_cfg Button configuration structure that triggered the event.
 * @retval None.
 * TODO: user can add actions on button events here.
 */
void on_button_longpress(btn_cfg_t *btn_cfg)
{
  bool setup_mode = is_setup_mode();

  if ((btn_cfg->gpio_port == B_SETUP_Port) && (btn_cfg->gpio_pin == B_SETUP_Pin))
  {
    // not in setup mode: enter setup mode and enable LCD backlight without timeout
    // in setup mode: exit setup mode and enable LCD backlight with timeout
    ctrl_lcd_backlight(true, setup_mode);
    set_setup_mode(!setup_mode);
  }
}

/**
 * @brief On release (short, repetitive or longpress) button callback.
 * @param *btn_cfg Button configuration structure that triggered the event.
 * @param state: button state before release event was triggered.
 * @retval None.
 * TODO: user can add actions on button events here.
 */
void on_button_release(btn_cfg_t *btn_cfg, btn_state_t state)
{
  if ((btn_cfg->gpio_port == B_SETUP_Port) && (btn_cfg->gpio_pin == B_SETUP_Pin))
  {
    if (state == BTN_STATE_LONGPRESS)
    {
      // printString("B1 press release");
    }
  }
}