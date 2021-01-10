/*
 * webusb.h
 *
 *  Created on: Oct 19, 2020
 *      Author: joris
 */

#ifndef INC_WEBUSB_H_
#define INC_WEBUSB_H_

#include "main.h"

void webusb_task();
void parseComputer(uint8_t *data, uint32_t len);
void parseBadge(uint8_t *data, uint32_t len);

#endif /* INC_WEBUSB_H_ */
