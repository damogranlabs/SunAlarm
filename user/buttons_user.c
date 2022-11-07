/*
 * Template library for handling button events.
 * @date    30-May-2020
 * @author  Domen Jurkovic
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs/Embedded-device-utilities-in-C/Embedded-device-utilities-in-C
 */
#include <stdint.h>

#include "buttons.h"
#include "buttons_user.h"

// add custom includes here to access required defines, cpu-specific LL functions, ...
#include "main.h"

#include "lcd.h"
#include "rot_enc.h"

#include "logic.h"
#include "sun_ctrl.h"
#include "display_ctrl.h"

extern rot_enc_data_t encoder;

/**
 * @brief Get current system milliseconds (systick) value (usually started at system reset).
 * @retval Milliseconds value.
 * TODO: user must implement this function to return milliseconds (systick) value.
 */
uint32_t btn_get_milliseconds(void)
{
  return GetTick();
}

/**
 * @brief Low level call to get actual pin state.
 * @param cfg: Registered button configuration structure.
 * @retval BTN_PHY_ACTIVE on button press, BTN_PHY_IDLE otherwise.
 * TODO: user must implement this function to return button state.
 */
btn_phy_state_t btn_get_pin_state(btn_cfg_t *cfg)
{
  uint32_t pin_state = LL_GPIO_IsInputPinSet(cfg->gpio_port, cfg->gpio_pin);

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
 * @brief On press (short, repetitive) button callback.
 * @param btn: Button configuration structure that triggered the event.
 * TODO: user can add actions on button events here.
 */
void btn_on_press(button_t *btn)
{
  // interactions and button handlings are described in logic.c -> handle_interactions()
  bool setup_mode = is_setup_mode();
  bool alarm_time_setup_mode = is_alarm_time_setup_mode();
  bool alarm_enabled = is_alarm_enabled();
  rot_enc_reset_count(&encoder);

  if ((btn->cfg.gpio_port == B_SETUP_Port) && (btn->cfg.gpio_pin == B_SETUP_Pin))
  {
    if (is_alarm_active())
    {
      set_alarm_active(false);
      ctrl_lcd_backlight(true, true);
    }
    else
    {
      if (setup_mode)
      {
        update_settings();
      }
      else
      {
        if (alarm_enabled)
        {
          set_alarm_time_setup_mode(false);
        }
        set_alarm_state(!alarm_enabled);
        ctrl_lcd_backlight(true, true);
      }
    }
  }
  else if ((btn->cfg.gpio_port == B_LA_CTRL_Port) && (btn->cfg.gpio_pin == B_LA_CTRL_Pin))
  {
    if (!setup_mode)
    {
      if (alarm_time_setup_mode)
      {
        set_alarm_time_setup_mode(false);
        ctrl_lcd_backlight(false, false);
      }
      else
      {
        if (is_lcd_backlight_enabled())
        {
          if (is_sun_enabled())
          {
            sun_pwr_off();
            if (is_alarm_timeout_pending())
            {
              reset_alarm_timeout_timestamp();
            }
            ctrl_lcd_backlight(false, false);
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
}

/**
 * @brief On press (long) button callback
 * @param btn Button configuration structure that triggered the event.
 * TODO: user can add actions on button events here.
 */
void btn_on_longpress(button_t *btn)
{
  // interactions and button handlings are described in logic.c -> handle_interactions()
  bool setup_mode = is_setup_mode();
  bool alarm_time_setup_mode = is_alarm_time_setup_mode();

  rot_enc_reset_count(&encoder);
  if ((btn->cfg.gpio_port == B_SETUP_Port) && (btn->cfg.gpio_pin == B_SETUP_Pin))
  {
    set_alarm_active(false);
    set_setup_mode(!setup_mode);
    ctrl_lcd_backlight(true, setup_mode);
  }
  else if ((btn->cfg.gpio_port == B_LA_CTRL_Port) && (btn->cfg.gpio_pin == B_LA_CTRL_Pin))
  {
    if (!(setup_mode || alarm_time_setup_mode))
    {
      if (is_alarm_enabled())
      {
        set_alarm_time_setup_mode(true);
        ctrl_lcd_backlight(true, false);
      }
    }
  }
}

/**
 * @brief On release (short, repetitive or longpress) button callback.
 * @param btn Button configuration structure that triggered the event.
 * TODO: user can add actions on button events here.
 */
void btn_on_release(button_t *btn)
{
  if ((btn->cfg.gpio_port == B_LA_CTRL_Port) && (btn->cfg.gpio_pin == B_LA_CTRL_Pin))
  {
  }
}