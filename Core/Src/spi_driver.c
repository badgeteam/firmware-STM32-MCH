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
#include <string.h>
#include "tusb.h"
#include "webusb.h"

#define MESSAGE_SIZE 18

#define VERSION 1
#define COM_SIZE 128

DMA_HandleTypeDef *hdma_spi_rx;
DMA_HandleTypeDef *hdma_spi_tx;

SPI_HandleTypeDef *spi_hw;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

volatile uint8_t tx_buffer[MESSAGE_SIZE];
volatile uint8_t rx_buffer[MESSAGE_SIZE];

uint8_t command_queue[COM_SIZE][8];
uint8_t comq_writeptr;
uint8_t comq_readptr;

GPIO_TypeDef *port_gpio[] = {SAO_IO0_GPIO_Port, SAO_IO1_GPIO_Port, SAO_IO2_GPIO_Port, SAO_IO3_GPIO_Port, EXT_IO0_GPIO_Port, EXT_IO1_GPIO_Port};
uint32_t pin_gpio[] = {SAO_IO0_Pin, SAO_IO1_Pin, SAO_IO2_Pin, SAO_IO3_Pin, EXT_IO0_Pin, EXT_IO1_Pin};

uint8_t status_reg[6];
uint8_t pinstate_prev;
uint8_t pinstate_mask;


void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *spi) {
	uint16_t *command = (uint16_t *) rx_buffer;


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
	{
		uint16_t *freq;
		uint16_t *duty;
		freq = (uint16_t *) &rx_buffer[2];
		set_spkr(*freq, rx_buffer[4]);
	}
		break;
	case 0x0004:	//Set LED
		set_pixel(rx_buffer[2], rx_buffer[3] << 24 | rx_buffer[4] << 16 | rx_buffer[5] << 8 | rx_buffer[6]);
		break;
	case 0x0005:	//ADC Sample
		if(rx_buffer[2] == 0) {
			HAL_ADC_DeInit(&hadc1);
			HAL_ADC_DeInit(&hadc2);
		} else if(rx_buffer[2] == 1) {
			HAL_ADC_Init(&hadc1);
			HAL_ADC_Init(&hadc2);
		} else if(rx_buffer[2] == 2) {
			HAL_ADC_Start_IT(&hadc1);
			HAL_ADC_Start_IT(&hadc2);
		}
		break;
	case 0x0006:	//Reset
		disable_spkr();
		clear_leds();
		break;
	case 0x0007:
		for(int i = 0; i < 5; i++) {
			GPIO_InitTypeDef GPIO_InitStruct = {0};
			GPIO_InitStruct.Pin = pin_gpio[i];
     	    GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			if(rx_buffer[i+2]) {
				GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
			} else {
				GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
			}
			HAL_GPIO_Init(port_gpio[i], &GPIO_InitStruct);
		}
		break;
	case 0x0008:
		for(int i = 0; i < 5; i++) {
			HAL_GPIO_WritePin(port_gpio[i], pin_gpio[i], rx_buffer[i+2]);
		}
		break;
	case 0x0009:
		pinstate_mask = rx_buffer[2];
		break;
	case 0xF001:
	case 0xF002:
	case 0xF003:
	case 0xF004:
	case 0xF005:
	case 0xF006:
	case 0xF007:
	case 0xF008:
	case 0xF009:
	case 0xF00A:
	case 0xF00B:
	case 0xF00C:
		handleBadge(&rx_buffer[2], rx_buffer[0]);
		break;
	}

	if(!readCommand(&tx_buffer[9])){
		memcpy(&tx_buffer[9], tx_buffer, 9);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1);
	} else {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0);
	}

	//HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
	//HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi) {
	//HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
}

void spi_reset_transmission() {
	HAL_SPI_Abort(spi_hw);
	HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
}

void spi_init(SPI_HandleTypeDef *spi) {
	spi_hw = spi;
	hdma_spi_rx = spi->hdmarx;
	hdma_spi_tx = spi->hdmatx;
	HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
	status_reg[5] = VERSION;

	comq_writeptr = 0;
	comq_readptr = 0;
	pinstate_mask = 0;
}

/*
 * Returns how much commands can still be stored in the command queue
 */
uint32_t commandSpace() {
	if(comq_writeptr == comq_readptr) return COM_SIZE;
	if(comq_writeptr < comq_readptr) {
		return (comq_readptr - comq_writeptr);
	}
	return COM_SIZE - (comq_writeptr - comq_readptr);
}

/*
 * Returns pointer to an 8-byte command. Only use this from the main thread
 */
uint8_t* getCommandSlot() {
	return command_queue[comq_writeptr];
}

/*
 * Indicate that the previous fetched command slot has been filled
 */
void commandReady() {
	comq_writeptr++;
	if(comq_writeptr == COM_SIZE) comq_writeptr = 0;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0);
}

uint32_t readCommand(uint8_t* buffer) {
	if(comq_writeptr == comq_readptr) {
		return 0;
	}
	memcpy(buffer, command_queue[comq_readptr], 8);
	comq_readptr++;
	if(comq_readptr == COM_SIZE) comq_readptr = 0;
	return 1;
}


void spi_update() {
	uint8_t pinstate = 0;
	for(int i = 0; i < 5; i++) {
		pinstate = (pinstate << 1) | HAL_GPIO_ReadPin(port_gpio[i], pin_gpio[i]);
	}
	status_reg[4] = pinstate | HAL_GPIO_ReadPin(LCD_REQUEST_GPIO_Port, LCD_REQUEST_Pin) << 6 | HAL_GPIO_ReadPin(LCD_MODE_GPIO_Port, LCD_MODE_Pin) << 7;

	if((pinstate ^ pinstate_prev) & pinstate_mask) {
		uint8_t* com = getCommandSlot();
		com[0] = 0x01;
		com[1] = 0x00;
		com[2] = (pinstate ^ pinstate_prev) & pinstate_mask;
		commandReady();
	}
	pinstate_prev = pinstate;

	memcpy((void *) &tx_buffer[2], (void *) status_reg, 6);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if(hadc == &hadc1) {
		uint16_t *V = (uint16_t *) &status_reg[0];
		*V = HAL_ADC_GetValue(hadc);
	} else {
		uint16_t *V = (uint16_t *) &status_reg[2];
		*V = HAL_ADC_GetValue(hadc);
	}
}
