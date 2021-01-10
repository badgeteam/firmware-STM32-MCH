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

#define BUFFER_SZ 512

tu_fifo_t webusb_tx_ff;		//Fifo for transmitting webusb packets towards the badge
tu_fifo_t webusb_rx_ff;		//Fifo for transmitting webusb packets towards the badge
tu_fifo_t local_ff;			//Fifo for receiving local commands handled by the stm32
tu_fifo_t uart_tx_ff;		//Fifo for transmitting uart data towards the badge
tu_fifo_t uart_rx_ff;		//Fifo for receving uart data from the badge

uint8_t webusb_tx_ff_buf[BUFFER_SZ];
uint8_t webusb_rx_ff_buf[BUFFER_SZ];
uint8_t local_ff_buf[BUFFER_SZ];
uint8_t uart_rx_buf[BUFFER_SZ];
uint8_t uart_tx_buf[BUFFER_SZ];

uint8_t buffer[512];


typedef struct __attribute__((packed)) {
	uint16_t command;
	uint32_t size;
	uint16_t ident;
	uint32_t id;
} messageheader_t;


void parseHeader(messageheader_t *header, uint8_t *payload) {
	header->command = *((uint16_t *) &payload[0]);
	header->size = *((uint32_t *) &payload[2]);
	header->ident = *((uint16_t *) &payload[6]);
	header->id = *((uint32_t *) &payload[8]);
}

void writeHeader(messageheader_t *header, uint8_t *payload) {
	 *((uint16_t *) &payload[0]) = header->command;
	 *((uint32_t *) &payload[2]) = header->size;
	 *((uint16_t *) &payload[6]) = header->ident;
	 *((uint32_t *) &payload[8]) = header->id;
}

void webusb_init() {
	tu_fifo_config(&webusb_tx_ff, webusb_tx_ff_buf, BUFFER_SZ, 1, false);
	tu_fifo_config(&webusb_rx_ff, webusb_rx_ff_buf, BUFFER_SZ, 1, false);
	tu_fifo_config(&local_ff, local_ff_buf, BUFFER_SZ, 1, false);
	tu_fifo_config(&uart_tx_ff, uart_tx_buf, BUFFER_SZ, 1, false);
	tu_fifo_config(&uart_rx_ff, uart_rx_buf, BUFFER_SZ, 1, false);
}

void webusb_task() {
	static uint32_t tx_status = 0;
	static uint32_t tx_toread = 0;
	static uint32_t rx_status = 0;
	static uint32_t rx_toread = 0;
	//receive data
	if(tu_fifo_remaining(&webusb_tx_ff) > 12 && tu_fifo_remaining(&local_ff) > 12) { //Check if both fifo's have space
		if(tx_status == 0) {
			if(tud_vendor_available() >= 12) {
				messageheader_t message;
				uint8_t header[12];
				tud_vendor_read(header, 12);
				parseHeader(&message, header);
				if (message.ident == 0xADDE) {
					tx_toread = message.size;
					tx_status = message.command < 12288 ? 1 : 2;	//Status 1 normal command, status 2 imm command
				}

				if(tx_status == 1) {
					tu_fifo_write_n(&webusb_tx_ff, header, 12);
				} else {
					tu_fifo_write_n(&local_ff, header, 12);
				}
			}
			if(tx_toread == 0) tx_status = 0;
		} else {
			uint32_t data_avail = min(tud_vendor_available(), tx_toread);
			if(tx_status == 1) {
				data_avail = min(data_avail, tu_fifo_remaining(&webusb_tx_ff));
				uint32_t dataread = tud_vendor_read(buffer, data_avail);
				tx_toread -= dataread;
				tu_fifo_write_n(&webusb_tx_ff, buffer, dataread);
			} else {
				data_avail = min(data_avail, tu_fifo_remaining(&local_ff));
				uint32_t dataread = tud_vendor_read(buffer, data_avail);
				tx_toread -= dataread;
				tu_fifo_write_n(&local_ff, buffer, dataread);
			}
			if(tx_toread == 0) tx_status = 0;
		}
	}

	 // connected and there are data available
	if ( tu_fifo_count(&webusb_tx_ff) && commandSpace() > 32) {
		 uint8_t *command = getCommandSlot();
		 command[1] = 0xF0;
		 uint16_t count = tu_fifo_read_n(&webusb_tx_ff, &command[2], 6);
		 command[0] = count;
		 commandReady();
	}

	//Send data
	uint32_t write_avail = tud_vendor_write_available();
	if(write_avail >= 12) {	 //Always have space for a message header
		if(rx_status == 0) {	//No transmission in progress
			if(tu_fifo_count(&webusb_rx_ff) >= 12) {	//Message header inside the webusb fifo
				messageheader_t message;
				uint8_t header[12];
				tu_fifo_read_n(&webusb_rx_ff, header, 12);
				parseHeader(&message, header);
				if(message.ident == 0xADDE) {	//Check checksum before transmission
					rx_toread = message.size;
					rx_status = 1;
					tud_vendor_write(header, 12);
				}

			} else if(tu_fifo_count(&uart_rx_ff)) {	//Message header inside the uart fifo
				messageheader_t message;
				message.command = 12289;
				message.id = 0;
				message.size = tu_fifo_count(&uart_rx_ff);
				message.ident = 0xADDE;
				rx_status = 2;
				rx_toread = message.size;
				uint8_t header[12];
				writeHeader(&message, header);
				tud_vendor_write(header, 12);
			}
			if(rx_toread == 0) rx_status = 0;
		} else if(rx_status == 1) {
			write_avail = min(write_avail, rx_toread);
			uint32_t numread = tu_fifo_read_n(&webusb_rx_ff, buffer, write_avail);
			rx_toread -= numread;
			tud_vendor_write(buffer, numread);
		} else if(rx_status == 2) {
			write_avail = min(write_avail, rx_toread);
			uint32_t numread = tu_fifo_read_n(&uart_rx_ff, buffer, write_avail);
			rx_toread -= numread;
			tud_vendor_write(buffer, numread);
		}
		if(rx_toread == 0) rx_status = 0;
	}

}

void handleBadge(uint8_t *data, uint32_t len) {
	tu_fifo_write_n(&webusb_rx_ff, data, len);
}

void handleUART(uint8_t *data, uint32_t len) {
	tu_fifo_write_n(&uart_rx_ff, data, len);
}

uint32_t availableUART() {
	return tu_fifo_count(&uart_tx_ff);
}

uint32_t readUART(uint8_t *data, uint32_t len) {
	return tu_fifo_read_n(&uart_tx_ff, data, len);
}
