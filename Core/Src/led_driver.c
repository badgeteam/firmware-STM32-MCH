/*
 * led_driver.c
 *
 *  Created on: Sep 27, 2020
 *      Author: joris
 */
#include "main.h"
#include "led_driver.h"

#define NUM_LEDS 6
#define LED0 60
#define LED1 30

#define BYTES_PER_LED 3
#define BITS_PER_LED BYTES_PER_LED*8
#define NUM_BITS NUM_LEDS*BYTES_PER_LED*8

uint32_t framebuffer[NUM_LEDS];
static uint16_t tmp_led_data[NUM_BITS];
volatile uint16_t led_data_bitpos;

TIM_HandleTypeDef *htim_used;
uint32_t TIM_Channel;

void init_leds(TIM_HandleTypeDef *htim, uint32_t Channel) {
    for(int i = 0; i < NUM_LEDS; i++) {
    	framebuffer[i] = 0x0000FF00;
    }
    htim_used = htim;
    TIM_Channel = Channel;
    HAL_TIM_Base_Start_IT(htim_used);

    update_leds();
}

void update_leds() {
	for(int i = 0; i < NUM_LEDS; i++) {
		uint32_t *alias_region = (uint32_t *) (((uint32_t) &framebuffer[i]-0x20000000)*32+0x22000000);
		for(int bitpos = 0; bitpos < BITS_PER_LED; bitpos++) {
			tmp_led_data[i*BITS_PER_LED+bitpos] = alias_region[31 - bitpos] ? LED0 : LED1;
		}
	}
	htim_used->Instance->CCR3 = tmp_led_data[0];
	led_data_bitpos = 1;
	HAL_TIMEx_PWMN_Start(htim_used, TIM_Channel);

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim == htim_used) {
		if(led_data_bitpos == NUM_BITS) {
			htim_used->Instance->CCR3 = 0;
			HAL_TIMEx_PWMN_Stop(htim_used, TIM_Channel);
			return;
		}
		htim_used->Instance->CCR3 = tmp_led_data[led_data_bitpos];
		led_data_bitpos++;
	}
}

