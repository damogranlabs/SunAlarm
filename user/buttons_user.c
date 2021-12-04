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

#include "setup_menu.h"

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
  if ((btn_cfg->gpio_port == B0_GPIO_Port) && (btn_cfg->gpio_pin == B0_Pin))
  {
    if (is_setup_mode())
    {
      update_settings();
    }
    else
    {
      set_alarm_state(!is_alarm_enabled());
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
  if ((btn_cfg->gpio_port == B0_GPIO_Port) && (btn_cfg->gpio_pin == B0_Pin))
  {
    set_setup_mode(!is_setup_mode());
  }
}
