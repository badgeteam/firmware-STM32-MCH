/*
 * webusb.h
 *
 *  Created on: Oct 19, 2020
 *      Author: joris
 */

#ifndef INC_WEBUSB_H_
#define INC_WEBUSB_H_

#include "main.h"
void webusb_init();
void webusb_task();

void handleBadge(uint8_t *data, uint32_t len);
void handleUART(uint8_t *data, uint32_t len);

uint32_t availableUART();
uint32_t readUART(uint8_t *data, uint32_t len);

#endif /* INC_WEBUSB_H_ */
