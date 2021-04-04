/*
 * uart_driver.c
 *
 *  Created on: Mar 18, 2020
 *      Author: joris
 */
#include "uart_driver.h"

/* Define size for the receive and transmit buffer over CDC */
#define APP_RX_DATA_SIZE  512
#define APP_TX_DATA_SIZE  512

uint8_t UserRxBufferFS[2][APP_RX_DATA_SIZE];
uint8_t UserTxBufferFS[2][APP_TX_DATA_SIZE];

uint32_t bufferpos[2];

uint32_t disablepins;
uint32_t disabletimeout;


extern UART_HandleTypeDef UART_SERIAL;
extern UART_HandleTypeDef UART_FPGA;

void ComPort_Config(void);

void uart_init(void) {
	//Override settings of ioc
	UART_SERIAL.Init.BaudRate = 115200;
	UART_SERIAL.Init.WordLength = UART_WORDLENGTH_8B;
	UART_SERIAL.Init.StopBits = UART_STOPBITS_1;
	UART_SERIAL.Init.Parity = UART_PARITY_NONE;
	UART_SERIAL.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART_SERIAL.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&UART_SERIAL) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	bufferpos[0] = 0;

	if (HAL_UART_Receive_DMA(&UART_SERIAL, (uint8_t*) UserTxBufferFS[0], APP_TX_DATA_SIZE / 2) != HAL_OK) {
		/* Transfer error in reception process */
		Error_Handler();
	}

	UART_FPGA.Init.BaudRate = 115200;
	UART_FPGA.Init.WordLength = UART_WORDLENGTH_8B;
	UART_FPGA.Init.StopBits = UART_STOPBITS_1;
	UART_FPGA.Init.Parity = UART_PARITY_NONE;
	UART_FPGA.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART_FPGA.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&UART_FPGA) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	bufferpos[1] = 0;

	if (HAL_UART_Receive_DMA(&UART_FPGA, (uint8_t*) UserTxBufferFS[1], APP_TX_DATA_SIZE / 2) != HAL_OK) {
		/* Transfer error in reception process */
		Error_Handler();
	}

}


void cdc_task(void)
{
    // connected and there are data available
    if ( tud_cdc_n_available(0) && UART_SERIAL.gState == HAL_UART_STATE_READY)
    {
      // read and echo back
      uint16_t count = tud_cdc_n_read(0, UserRxBufferFS[0], APP_RX_DATA_SIZE);
      if(HAL_UART_Transmit_DMA(&UART_SERIAL, UserRxBufferFS[0], count) != HAL_OK) {
    	  count++;
      }

    }
    tud_cdc_n_write_flush(0);
    if(disablepins && HAL_GetTick() > disabletimeout) {
    	disablepins = 0;
		HAL_GPIO_WritePin(ESP32_EN_GPIO_Port, ESP32_EN_Pin, 1);
		HAL_GPIO_WritePin(ESP32_BL_GPIO_Port, ESP32_BL_Pin, 1);
    }

    if ( tud_cdc_n_available(1) && UART_FPGA.gState == HAL_UART_STATE_READY)
        {
          // read and echo back
          uint16_t count = tud_cdc_n_read(1, UserRxBufferFS[1], APP_RX_DATA_SIZE);
          if(HAL_UART_Transmit_DMA(&UART_FPGA, UserRxBufferFS[1], count) != HAL_OK) {
        	  count++;
          }

        }
	tud_cdc_n_write_flush(1);


}

// Invoked when received new data
void tud_cdc_rx_cb(uint8_t itf) {

}

// Invoked when received `wanted_char`
void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {

}

// Invoked when line state DTR & RTS are changed via SET_CONTROL_LINE_STATE
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
	if(itf == 0) {
		if((dtr && rts)) {
			disablepins = 1;
			disabletimeout = HAL_GetTick()+10;
		} else {
			disablepins = 0;
		}
		HAL_GPIO_WritePin(ESP32_EN_GPIO_Port, ESP32_EN_Pin, !rts);
		HAL_GPIO_WritePin(ESP32_BL_GPIO_Port, ESP32_BL_Pin, !dtr);
	}
}

// Invoked when line coding is change via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
//		if (HAL_UART_DeInit(&UART_SERIAL) != HAL_OK) {
//			/* Initialization Error */
//			Error_Handler();
//		}
		UART_HandleTypeDef *uart;
		if(itf == 0)
			uart = &UART_SERIAL;
		else
			uart = &UART_FPGA;

		HAL_UART_AbortReceive(uart);

		/* set the Stop bit */
		switch (p_line_coding->stop_bits) {
		case 0:
			uart->Init.StopBits = UART_STOPBITS_1;
			break;
		case 2:
			uart->Init.StopBits = UART_STOPBITS_2;
			break;
		default:
			uart->Init.StopBits = UART_STOPBITS_1;
			break;
		}

		/* set the parity bit*/
		switch (p_line_coding->parity) {
		case 0:
			uart->Init.Parity = UART_PARITY_NONE;
			break;
		case 1:
			uart->Init.Parity = UART_PARITY_ODD;
			break;
		case 2:
			uart->Init.Parity = UART_PARITY_EVEN;
			break;
		default:
			uart->Init.Parity = UART_PARITY_NONE;
			break;
		}

		/*set the data type : only 8bits and 9bits is supported */
		switch (p_line_coding->data_bits) {
		case 0x07:
			/* With this configuration a parity (Even or Odd) must be set */
			uart->Init.WordLength = UART_WORDLENGTH_8B;
			break;
		case 0x08:
			if (uart->Init.Parity == UART_PARITY_NONE) {
				uart->Init.WordLength = UART_WORDLENGTH_8B;
			} else {
				uart->Init.WordLength = UART_WORDLENGTH_9B;
			}

			break;
		default:
			uart->Init.WordLength = UART_WORDLENGTH_8B;
			break;
		}


		uart->Init.BaudRate = p_line_coding->bit_rate;
		uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
		uart->Init.Mode = UART_MODE_TX_RX;



		HAL_UART_Init(uart);

		/* Start reception: provide the buffer pointer with offset and the buffer size */
		bufferpos[itf] = 0;
		HAL_UART_Receive_DMA(uart, (uint8_t*) UserTxBufferFS[itf], APP_TX_DATA_SIZE / 2);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart == &UART_SERIAL) {
		bufferpos[0] = !bufferpos[0];
		if (HAL_UART_Receive_DMA(&UART_SERIAL, (uint8_t*) &UserTxBufferFS[0][bufferpos[0]*APP_TX_DATA_SIZE/2],
					APP_TX_DATA_SIZE / 2) != HAL_OK) {
			Error_Handler();
		}
		tud_cdc_n_write(0, &UserTxBufferFS[0][!bufferpos[0]*APP_TX_DATA_SIZE/2+APP_TX_DATA_SIZE/4], APP_TX_DATA_SIZE/4); //Invert bufferpos again because we inverted it for the receive call
	} else if(huart == &UART_FPGA) {
		bufferpos[1] = !bufferpos[1];
		if (HAL_UART_Receive_DMA(&UART_SERIAL, (uint8_t*) &UserTxBufferFS[1][bufferpos[1]*APP_TX_DATA_SIZE/2],
					APP_TX_DATA_SIZE / 2) != HAL_OK) {
			Error_Handler();
		}
		tud_cdc_n_write(1, &UserTxBufferFS[1][!bufferpos[1]*APP_TX_DATA_SIZE/2+APP_TX_DATA_SIZE/4], APP_TX_DATA_SIZE/4); //Invert bufferpos again because we inverted it for the receive call
	}
}

void UART_Early_Exit(UART_HandleTypeDef *huart, uint32_t CNDTR)  {
	if(huart == &UART_SERIAL) {
			bufferpos[0] = !bufferpos[0];
			if (HAL_UART_Receive_DMA(&UART_SERIAL, (uint8_t*) &UserTxBufferFS[0][bufferpos[0]*APP_TX_DATA_SIZE/2],
						APP_TX_DATA_SIZE / 2) != HAL_OK) {
				Error_Handler();
			}
			uint32_t len = APP_TX_DATA_SIZE/2 - CNDTR; //Received number of bytes by the DMA
			uint32_t offset = len > APP_TX_DATA_SIZE/4 ? APP_TX_DATA_SIZE/4 : 0;
			len = len % (APP_TX_DATA_SIZE/4); //Remove the Half way callback

			tud_cdc_n_write(0, &UserTxBufferFS[0][!bufferpos[0]*APP_TX_DATA_SIZE/2+offset], len); //Invert bufferpos again because we inverted it for the receive call
		} else if(huart == &UART_FPGA) {
			bufferpos[1] = !bufferpos[1];
			if (HAL_UART_Receive_DMA(&UART_FPGA, (uint8_t*) &UserTxBufferFS[1][bufferpos[1]*APP_TX_DATA_SIZE/2], APP_TX_DATA_SIZE / 2) != HAL_OK) {
				Error_Handler();
			}
			uint32_t len = APP_TX_DATA_SIZE/2 - CNDTR; //Received number of bytes by the DMA
			uint32_t offset = len > APP_TX_DATA_SIZE/4 ? APP_TX_DATA_SIZE/4 : 0;
			len = len % (APP_TX_DATA_SIZE/4); //Remove the Half way callback

			tud_cdc_n_write(1, &UserTxBufferFS[1][!bufferpos[1]*APP_TX_DATA_SIZE/2+offset], len); //Invert bufferpos again because we inverted it for the receive call
		}
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
	if(huart == &UART_SERIAL) {
			tud_cdc_n_write(0, &UserTxBufferFS[0][bufferpos[0]*APP_TX_DATA_SIZE/2], APP_TX_DATA_SIZE/4);
		} else if(huart == &UART_FPGA) {
			tud_cdc_n_write(1, &UserTxBufferFS[1][bufferpos[1]*APP_TX_DATA_SIZE/2], APP_TX_DATA_SIZE/4);
		}
}

void UART_Reset() {
	bufferpos[0] = 0;
	if (HAL_UART_Receive_DMA(&UART_SERIAL, (uint8_t*) &UserTxBufferFS[0][bufferpos[0]*APP_TX_DATA_SIZE/2],
					APP_TX_DATA_SIZE / 2) != HAL_OK) {
			Error_Handler();
	}
}

void FPGA_Reset() {
	bufferpos[1] = 0;
	if (HAL_UART_Receive_DMA(&UART_FPGA, (uint8_t*) &UserTxBufferFS[1][bufferpos[1]*APP_TX_DATA_SIZE/2],
					APP_TX_DATA_SIZE / 2) != HAL_OK) {
			Error_Handler();
	}
}



