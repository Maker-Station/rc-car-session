/*
 * Thunder_firmware_v1.0.c
 *
 * Created: 10/12/2019 7:20:47 PM
 * Author : workstation
 */ 
#define  F_CPU	4000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "lib_usart.h"



int main(void){
	
	/* initialize the serial block */
	struct usart_config_t config;
	lib_usart_get_defaults(&config);
	config.boudrate = 19200;
	config.mode =  USART_ASYNCH_MODE;
	config.dFrameSize = USART_DATA_FRAME_8x;
	config.parityMode = USART_PARITY_DISABLE;
	config.stopBits = USART_STOPBITS_1x;
	config.rxEnable = true;
	config.txEnable = true;
	if(lib_usart_init(&config)){				//initialize the serial port
		while(1);
	}
	
	char str[] = "hello Atmega 328p!\n";
	char input;
	
	lib_uart_write_string(str);
	
	while (1) {
		//lib_uart_write_string(str);
		//lib_uart_write_char((char)10);
		//lib_uart_write_char((char)13);
		//_delay_ms(1000);
		if( lib_usart_read_char(&input) == 0){
			lib_uart_write_char(input+1);		
		}
    }
	
	return 0;
}




