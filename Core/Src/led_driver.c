/*
 * led_driver.c
 *
 *  Created on: Sep 27, 2020
 *      Author: joris
 */
#include "main.h"
#include "led_driver.h"
#include "stm32f1xx_it.h"

#define NUM_LEDS 6
#define LED0 60
#define LED1 30

#define BYTES_PER_LED 4
#define BITS_PER_LED BYTES_PER_LED*8
#define NUM_BITS NUM_LEDS*BYTES_PER_LED*8

uint32_t framebuffer[NUM_LEDS];
static uint16_t tmp_led_data[NUM_BITS+1];

TIM_HandleTypeDef *htim_used;
extern DMA_HandleTypeDef hdma_tim4_ch1;
extern TIM_HandleTypeDef htim4;
uint32_t TIM_Channel;

static void halfTransfer() {

}

static void fullTransfer() {
}

void set_pixel(uint32_t pixel, uint32_t data) {
	if(pixel < NUM_LEDS) {
		framebuffer[pixel] = data;
	}
}

void init_leds(TIM_HandleTypeDef *htim, uint32_t Channel) {
	htim_used = htim;
	TIM_Channel = Channel;

	for(int i = 0; i < NUM_LEDS; i++) {
    	framebuffer[i] = 0x00000000;
    }


	hdma_tim4_ch1.XferCpltCallback = fullTransfer;
	hdma_tim4_ch1.XferHalfCpltCallback = halfTransfer;

    HAL_DMA_Init(&hdma_tim4_ch1);


    htim_used->Instance->CCR3 = 0;
    HAL_TIM_Base_Start(htim_used);
    HAL_TIM_Base_Start(&htim4);
    if(HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) != HAL_OK) HardFault_Handler();
    if(HAL_TIMEx_PWMN_Start(htim_used, TIM_Channel) != HAL_OK) HardFault_Handler();

    tmp_led_data[NUM_BITS] = 0; //Blank PWM after transmission
    update_leds();
}

void update_leds() {
	for(int i = 0; i < NUM_LEDS; i++) {
		//framebuffer[i] = framebuffer[i] << 8;
		//if(framebuffer[i] == 0) framebuffer[i] = 0x000000FF;
		uint32_t *alias_region = (uint32_t *) (((uint32_t) &framebuffer[i]-0x20000000)*32+0x22000000);
		for(int bitpos = 0; bitpos < BITS_PER_LED; bitpos++) {
			tmp_led_data[i*BITS_PER_LED+bitpos] = alias_region[31 - bitpos] ? LED0 : LED1;
		}
	}

    HAL_DMA_Start_IT(&hdma_tim4_ch1, (uint32_t)tmp_led_data, (uint32_t)&(htim_used->Instance->CCR3), NUM_BITS+1);

    /* Enable the TIM Capture/Compare 1 DMA request */
    __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_CC1);

}

void clear_leds() {
	for(int i = 0; i < NUM_LEDS; i++) {
		framebuffer[0] = 0;
	}
	update_leds();
}


