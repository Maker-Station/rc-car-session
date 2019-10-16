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

/* RX call back function  */
void blinkOnRecieve(void){
	PORTC ^= 0xff;
}


int main(void){
	_delay_ms(500);		//cold start delay
	
	/* initialize the serial block */
	struct usart_config_t config;
	lib_usart_get_defaults(&config);
	config.boudrate = 19200;
	config.mode =  USART_ASYNCH_MODE;
	config.dFrameSize = USART_DATA_FRAME_8x;
	config.parityMode = USART_PARITY_DISABLE;
	config.stopBits = USART_STOPBITS_1x;
	config.txBuffer_size = 80;
	config.rxCallback = blinkOnRecieve;			//set the callback function
	if(lib_usart_init(&config)){				//initialize the serial port
		while(1);
	}
	
	char str1[] = "hello Atmega 328p!";
	int counter  = 0;
	
	while (1) {
		
		/* uncomment for read test */
		while( lib_usart_available() ){
			lib_usart_write_byte(lib_usart_read()+1);
		}
		
		/* */
		if(++counter == 10){
			lib_usart_write((uint8_t*)str1, strlen(str1));
			lib_usart_write_byte(10);
			lib_usart_write_byte(13);
			counter = 0;
			_delay_ms(900);
		}
		
		_delay_ms(100);
    }
	
	return 0;
}




