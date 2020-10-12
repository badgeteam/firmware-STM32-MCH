/*
 * led_driver.h
 *
 *  Created on: Sep 27, 2020
 *      Author: joris
 */

#ifndef INC_LED_DRIVER_H_
#define INC_LED_DRIVER_H_

void init_leds(TIM_HandleTypeDef *htim, uint32_t Channel);
void update_leds();
void set_pixel(uint32_t pixel, uint32_t data);
void clear_leds();

#endif /* INC_LED_DRIVER_H_ */
