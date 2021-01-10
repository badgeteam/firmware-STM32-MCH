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

#define min(X,Y) ((X) < (Y) ? (X) : (Y))

tu_fifo_t webusb_ff;
tu_fifo_t local_ff;
tu_fifo_t uart_tx_ff;
tu_fifo_t uart_rx_ff;

uint8_t webusb_ff_buf[512];
uint8_t local_ff_buf[512];
uint8_t uart_rx_buf[512];
uint8_t uart_tx_buf[512];


typedef struct __attribute__((packed)) {
	uint16_t command;
	uint32_t size;
	uint16_t ident;
	uint32_t id;
} messageheader_t;


void webusb_init() {
	tu_fifo_config(&webusb_ff, webusb_ff_buf, 512, 1, false);
	tu_fifo_config(&local_ff, local_ff_buf, 512, 1, false);
	tu_fifo_config(&uart_tx_ff, uart_tx_buf, 512, 1, false);
	tu_fifo_config(&uart_rx_ff, uart_rx_buf, 512, 1, false);
}

void webusb_task() {

	static uint32_t status = 0;
	static uint32_t toread = 0;
	if(tu_fifo_remaining(&webusb_ff) > 12 && tu_fifo_remaining(&local_ff) > 12) { //Check if both fifo's have space
		if(status == 0) {
			if(tud_vendor_available() >= 12) {
				messageheader_t message;
				tud_vendor_read(&message, 12);
				if (message.ident == 0xEDDA) {
					toread = message.size;
					status = message.command < 12288 ? 1 : 2;	//Status 1 normal command, status 2 imm command
				}

				if(status == 1) {
					tu_fifo_write_n(&webusb_ff, &message, 12);
				} else {
					tu_fifo_write_n(&local_ff, &message, 12);
				}
			}
		} else {
			uint32_t data_avail = min(tud_vendor_available(), toread);
			if(status == 1) {
				data_avail = min(data_avail, tu_fifo_remaining(&webusb_ff));
				uint8_t data[data_avail];
				uint32_t dataread = tud_vendor_read(data, data_avail);
				toread -= dataread;
				tu_fifo_write_n(&webusb_ff, data, dataread);
			} else {
				data_avail = min(data_avail, tu_fifo_remaining(&local_ff));
				uint8_t data[data_avail];
				uint32_t dataread = tud_vendor_read(data, data_avail);
				toread -= dataread;
				tu_fifo_write_n(&local_ff, data, dataread);
			}
			if(toread == 0) status = 0;
		}
	}

	 // connected and there are data available
	if ( tu_fifo_count(&webusb_ff) && commandSpace() > 32) {
		 uint8_t *command = getCommandSlot();
		 command[1] = 0xF0;
		 uint16_t count = tud_vendor_read(&command[2], 6);
		 command[0] = count;
		 commandReady();
	}

}

void handleBadge(uint8_t *data, uint32_t len) {

}

void handleUART(uint8_t *data, uint32_t len) {

}
