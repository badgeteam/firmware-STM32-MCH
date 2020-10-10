/*
 * spi_driver.c
 *
 *  Created on: Oct 9, 2020
 *      Author: joris
 */

#include "spi_driver.h"
#include "main.h"

#define MESSAGE_SIZE 18

SPI_HandleTypeDef *spi_hw;

uint8_t tx_buffer[MESSAGE_SIZE];
uint8_t rx_buffer[MESSAGE_SIZE];

void spi_init(SPI_HandleTypeDef *spi) {
	spi_hw = spi;


}

void exti_handler(uint32_t pinstate) {
	if(pinstate == 0) { //Start of transmission
		HAL_SPI_TransmitReceive_DMA(spi_hw, tx_buffer, rx_buffer, MESSAGE_SIZE);
	}
}
