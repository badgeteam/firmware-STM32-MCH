/*
 * spkr_helper.c
 *
 *  Created on: Oct 11, 2020
 *      Author: joris
 */

#include "spkr_helper.h"

#define FREQUENCY_ICLK 3272727

extern TIM_HandleTypeDef htim2;

void disable_spkr() {
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}

void set_spkr(uint16_t frequency, uint8_t duty) {
	uint16_t period = FREQUENCY_ICLK/frequency;
	uint16_t duty_cycle = period*duty>>8;
	HAL_TIM_Base_Stop(&htim2);
	htim2.Init.Period = period;

	TIM_OC_InitTypeDef sConfigOC = {0};

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = duty_cycle;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}
