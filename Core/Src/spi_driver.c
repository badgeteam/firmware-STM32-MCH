/*
 * spi_driver.c
 *
 *  Created on: Oct 9, 2020
 *      Author: joris
 */

#include "spi_driver.h"
#include "main.h"
#include "lcd_helper.h"
#include "spkr_helper.h"
#include "led_driver.h"

#define MESSAGE_SIZE 18

DMA_HandleTypeDef *hdma_spi_rx;
DMA_HandleTypeDef *hdma_spi_tx;

SPI_HandleTypeDef *spi_hw;

volatile uint8_t tx_buffer[MESSAGE_SIZE];
volatile uint8_t rx_buffer[MESSAGE_SIZE];

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *spi) {
	uint16_t *command = (uint16_t *) rx_buffer;
	uint16_t *freq;
	uint16_t *duty;

	switch(*command) {
	case 0x0000:

		break;
	case 0x0001:	//LCD Mode switch
		if(rx_buffer[2]) {	//Enable Parallel mode
			Enable_parallel_mode();
		} else {	//Enable spi mode
			Enable_spi_mode();
		}
		break;
	case 0x0002:	//LCD Backlight
		Set_brightness(rx_buffer[2]);
		break;
	case 0x0003:	//Set speaker
		freq = (uint16_t *) &rx_buffer[2];
		duty = (uint16_t *) &rx_buffer[4];
		set_spkr(*freq, *duty);
		break;
	case 0x0004:	//Set LED
		set_pixel(rx_buffer[2], rx_buffer[3] << 24 | rx_buffer[4] << 16 | rx_buffer[5] << 8 | rx_buffer[6]);
		break;
	case 0x0005:	//ADC Sample
		if(rx_buffer[2] == 0) {

		} else if(rx_buffer[2] == 1) {

		} else if(rx_buffer[2] == 2) {

		}
		break;
	case 0x0006:	//Reset
		disable_spkr();
		clear_leds();
		break;
	case 0x0007:

		break;
	case 0x0008:

		break;
	case 0x0009:

		break;
	case 0xF000:

		break;
	case 0xF001:

		break;
	}
}

void spi_init(SPI_HandleTypeDef *spi) {
	spi_hw = spi;
	hdma_spi_rx = spi->hdmarx;
	hdma_spi_tx = spi->hdmatx;

}

void exti_handler(uint32_t pinstate) {
	if(pinstate == 0) { //Start of transmission
		HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
	} else {
		HAL_SPI_Abort(spi_hw);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
	if(pin == CS_Pin) {
		exti_handler(HAL_GPIO_ReadPin(CS_GPIO_Port, CS_Pin));
	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {

}
