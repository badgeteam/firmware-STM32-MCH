/*
 * lcd_helper.c
 *
 *  Created on: Oct 11, 2020
 *      Author: joris
 */
#include "main.h"
#include "lcd_helper.h"

uint32_t resettime;

extern TIM_HandleTypeDef htim4;

void Enable_parallel_mode() {
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, 0);
	HAL_GPIO_WritePin(LCD_MODE_GPIO_Port, LCD_MODE_Pin, 1);
	resettime = HAL_GetTick() + 2;
}

void Enable_spi_mode() {
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, 0);
	HAL_GPIO_WritePin(LCD_MODE_GPIO_Port, LCD_MODE_Pin, 0);
	resettime = HAL_GetTick() + 2;
}

void Set_brightness(uint8_t brightness) {
	htim4.Instance->CCR4 = 90*brightness>>8;
}

void init_lcd() {
	Enable_spi_mode();
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

void lcd_helper() {
	if(HAL_GetTick() >= resettime) {
		HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, 1);
	}
}
