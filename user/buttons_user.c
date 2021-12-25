/*
 * Template library for handling button events.
 * @date    30-May-2020
 * @author  Domen Jurkovic
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs
 */
#include "stdint.h"

#include "buttons.h"
#include "buttons_user.h"

// add custom includes here to access required defines, cpu-specific LL functions, ...
#include "main.h"

// TODO
#include "lcd.h"

#include "logic.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"

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
  if (is_alarm_active())
  {
    set_alarm_active(false);
    ctrl_lcd_backlight(true, true);
  }
  else
  {
    // use any button to cancel alarm
    if ((btn_cfg->gpio_port == B_SETUP_Port) && (btn_cfg->gpio_pin == B_SETUP_Pin))
    {
      if (is_setup_mode())
      {
        update_settings();
      }
      else
      {
        set_alarm_state(!is_alarm_enabled());
        ctrl_lcd_backlight(true, true);
      }
    }
    else if ((btn_cfg->gpio_port == B_LA_CTRL_Port) && (btn_cfg->gpio_pin == B_LA_CTRL_Pin))
    {
      // use this button to only turn on LCD backlight (see time/alarm).
      // Press again in this time to power on sun (manual intensity)
      // Power off after next button press (LCd after timeout)
      if (is_sun_enabled())
      {
        sun_pwr_off();
        ctrl_lcd_backlight(true, true);
      }
      else
      {
        if (is_lcd_backlight_enabled())
        {
          sun_pwr_on_manual();
          ctrl_lcd_backlight(true, false);
        }
        else
        {
          ctrl_lcd_backlight(true, true);
        }
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
    if (setup_mode)
    {
      ctrl_lcd_backlight(true, true);
    }
    else
    {
      ctrl_lcd_backlight(true, false);
    }

    set_setup_mode(!setup_mode);
  }
}
