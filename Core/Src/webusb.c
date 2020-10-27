/*
 * webusb.c
 *
 *  Created on: Oct 19, 2020
 *      Author: joris
 */
#include "main.h"
#include "spi_driver.h"
#include "tusb.h"
#include "webusb.h"

void webusb_task() {
	 // connected and there are data available
	    if ( tud_vendor_available() && commandSpace() > 32) {
	     uint8_t *command = getCommandSlot();
	     command[0] = 0x00;
	     command[1] = 0xF0;
	     uint16_t count = tud_vendor_read(&command[2], 6);
	     command[0] = count;
	     commandReady();
	    }
}
