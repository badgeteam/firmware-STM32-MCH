/*
 * lcd_helper.h
 *
 *  Created on: Oct 11, 2020
 *      Author: joris
 */

#ifndef INC_LCD_HELPER_H_
#define INC_LCD_HELPER_H_
#include "main.h"

void Enable_parallel_mode();
void Enable_spi_mode();
void lcd_helper();
void Set_brightness(uint8_t brightness);
void init_lcd();


#endif /* INC_LCD_HELPER_H_ */
