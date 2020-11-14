/*
 * spi_driver.h
 *
 *  Created on: Oct 9, 2020
 *      Author: joris
 */

#ifndef INC_SPI_DRIVER_H_
#define INC_SPI_DRIVER_H_
#include "main.h"

void spi_init(SPI_HandleTypeDef *spi);
void spi_update();
void spi_reset_transmission();

uint32_t commandSpace();
uint8_t* getCommandSlot();
void commandReady();
uint32_t readCommand(uint8_t* buffer);


#endif /* INC_SPI_DRIVER_H_ */
