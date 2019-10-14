/*
 * lib_usart.c
 *
 * Created: 10/12/2019 8:13:53 PM
 *  Author: workstation
 */ 
#include <avr/io.h>
#include <string.h>
#include <stdbool.h>
#include <avr/sfr_defs.h>
#include "lib_usart.h"
#include "sys_config.h"

/***************** GLOBAL VARIABLES ************************/
bool dFrameis9x = false;


/*
@description -
	set the (param)cfg to known settings as below.

@param -
	struct usart_config_t *cfg - pointer to configurations

@return -
	
*/
void lib_usart_get_defaults(struct usart_config_t *cfg){
	cfg->boudrate = 9600;
	cfg->dFrameSize = USART_DATA_FRAME_8x;
	cfg->mode = USART_ASYNCH_MODE;
	cfg->parityMode = USART_PARITY_DISABLE;
	cfg->stopBits = USART_STOPBITS_1x;
	cfg->rxBuffer_size = cfg->txBuffer_size = 1;
	
	cfg->txEnable = false;
	cfg->rxEnable = false;
}



/*
@description -
	initialize the the USART with given configuration.
	
@param -
	struct usart_config_t *cfg - pointer to configurations

@return -
	0 if configuration is successfully
	1 if failed
*/
int lib_usart_init(struct usart_config_t *cfg){
	uint16_t bout_reg_value = 0;
	
	/* disable usart */
	UCSR0B  &= ~( _BV(TXEN0) | _BV(RXEN0) );
	
	/* set usart mode*/ 
	UCSR0C = (UCSR0C &  ~(_BV(UMSEL00) | _BV(UMSEL01)) ) 
			 | cfg->mode; 
	switch(cfg->mode){
		case USART_ASYNCH_MODE:
			bout_reg_value = ( ((uint32_t)F_CPU/16)/cfg->boudrate) -1; //baud rate
			
			/* TODO -- auto determine the clock rate based on current F_CPU 
				return ERROR if given F_CPU is not capable to config the given boud rate
			*/
			
			break;
		case USART_SYNCH_MODE:
			UCSR0C = ( UCSR0C &  ~_BV(UCPOL0) )							//set SYNCH mode clock polarity
			| cfg->synchModeEdge;
			
			bout_reg_value = ( ((uint32_t)F_CPU/2)/cfg->boudrate) -1;	//baud rate
		
		case USART_MASTER_SPI_MODE:
			bout_reg_value = ( ((uint32_t)F_CPU/2)/cfg->boudrate) -1;	//baud rate
		default:
			return 1;
	}
	
	/* set boudrate register */
	UBRR0 = bout_reg_value;
	
	/* set data frame size */		
	if(cfg->dFrameSize != USART_DATA_FRAME_9x){
		UCSR0C = ( UCSR0C & ~ (_BV(UCSZ00) | _BV(UCSZ01)) ) 
				| cfg->dFrameSize;
	}else{
		UCSR0C = ( UCSR0C & ~(_BV(UCSZ00) | _BV(UCSZ01)) ) | 0x06;
		UCSR0B |= _BV(UCSZ02);
		
		dFrameis9x = true;	//indicates that data frame is 9 bit
	}
	
	/* set the stop bit size */
	UCSR0C = (UCSR0C & ~_BV(USBS0)) 
			 | cfg->stopBits;
	
	/* set the parity */
	UCSR0C = (UCSR0C & ~(_BV(UPM01) | _BV(UPM00)) )
			 | cfg->parityMode; 

	/* enable RX */
	if(cfg->rxEnable){
		UCSR0B |= _BV(RXEN0);
	}
	
	/* enable TX */
	if(cfg->txEnable){
		UCSR0B |= _BV(TXEN0);
	}
	
	return 0;
}


//write one byte to serial port
int lib_uart_write_char(char data){
	while( !(UCSR0A & 0x20));			//wait until buffer empty
	UDR0 = data;	
	
	
	return 0;
}




int lib_uart_write_string(char str[]){
	int len = strlen(str);
	int i = 0;
	while(len--){
		lib_uart_write_char(str[i++]);
	}	
}



/*
@description -
	read most imidiate value in UDR registers
	
@param -

@return -
	0 - if success
	1 - if error
*/
int lib_usart_read_char(char *ch){
	int count = 1 ;
	
	while( !(UCSR0A & _BV(RXC0)) ){
		count ++;
		if (count == 100) 
			return 1;			
	}
	
	*ch = UDR0;
	return 0;
}


