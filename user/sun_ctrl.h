/*
 * sun_ctrl.h
 *
 *  Created on: 25 Apr 2021
 *      Author: domen
 */

#ifndef SUN_CTRL_H_
#define SUN_CTRL_H_

#include <stdint.h>

void sun_init(void);

void sun_pwr_on(void);
void sun_pwr_off(void);

void sun_set_indensity(uint16_t intensity);


#endif /* SUN_CTRL_H_ */
