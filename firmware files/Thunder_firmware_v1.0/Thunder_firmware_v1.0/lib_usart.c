/*
 * lib_usart.c
 *
 * Created: 10/12/2019 8:13:53 PM
 *  Author: workstation
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/sfr_defs.h>
#include "lib_usart.h"
#include "sys_config.h"
#include "circular_buffer.h"


/***************** GLOBAL VARIABLES ************************/
bool dFrameis9x = false;
usart_callback_t rxCallback = NULL;
usart_callback_t txCallback = NULL;

uint8_t* rxBuffer = NULL;
cbuf_handle_t rxRingBuff;				// ring buffer for rx data

uint8_t* txBuffer = NULL;
cbuf_handle_t txRingBuff;				// ring buffer for tx data


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
	
	cfg->rxBuffer_size = USART_RX_BUFFER_SIZE;
	cfg->txBuffer_size = USART_TX_BUFFER_SIZE;

	cfg->rxCallback = NULL;
	cfg->txCallback = NULL;	
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
	
	/* disable usart - set to default shutdown mode */
	UCSR0A = 0x40;
	UCSR0B = 0x00;
	UCSR0C = 0x06;
	
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


		
	/* allocate buffers for RX data */
	rxBuffer = (uint8_t*)malloc(sizeof(uint8_t)* cfg->rxBuffer_size);
	if(rxBuffer == NULL){
		return 1;
	}
	rxRingBuff = circular_buf_init(rxBuffer, cfg->rxBuffer_size);

	/* allocate buffers for RX data */
	txBuffer = (uint8_t*)malloc(sizeof(uint8_t)* cfg->txBuffer_size);
	if(txBuffer == NULL){
		return 1;
	}
	txRingBuff = circular_buf_init(txBuffer, cfg->txBuffer_size);

	/* set callback function for RX */
	if(cfg->rxCallback != NULL)
		rxCallback = cfg->rxCallback;
	
	/* TODO -- SET CALLBACK FOR TX */
	
	/* enable RX and TX along with interrupts */
	UCSR0B |= ( _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) /*| _BV(TXCIE0)*/);
	sei();
	
	return 0;
}


/*
@description -
	write byte to usart port
	
@param -
	uint8_t data - byte data being write
@return -
	0 - if success
	1 - if error
*/
int lib_usart_write_byte(uint8_t data){
	if(circular_buf_full(txRingBuff))
		return 1;
	
	circular_buf_put2(txRingBuff, data);
	UCSR0B |= _BV(UDRIE0);				// enable buffer empty interrupt to continue the operation
	return 0;
}




/*
@description -
	write byte array to usart port
	
@param -
	uint8_t *data - pointer to data being write
	uint8_t len - length of the data array
@return -
	0 - if success
	1 - if error
*/
int lib_usart_write(uint8_t* data, uint8_t len){
	uint8_t index = 0;
	
	while(index < len ){
		if(circular_buf_full(txRingBuff))
			return 1;
			
		circular_buf_put2(txRingBuff, data[index++]);
	}
	
	UCSR0B |= _BV(UDRIE0);				// enable buffer empty interrupt to continue the operation
	return 0;
}





/*
@description -
	read most immediate value in UDR registers
	
@param -
	char* - pointer to data being read
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


/*
@description -
	this function checks rx buffer for data availability. returns true if there
are unread data.
	
@param -
	void
@return -
	true  - if data available
	false - if no data in rx buffer
*/
bool lib_usart_available(){
	if(circular_buf_size(rxRingBuff) > 0)
		return true;
	
	return false;
}


/*
@description -
	return the  most leading byte available in the rx ring buffer.
	
	! Note *! - this function return 0 if there are no data in the buffer.
	use lib_usart_available() before calling this function to verify
	data is available.
@param -
	void
@return -
	char - data byte. 
	
*/
char lib_usart_read(){
	uint8_t data;
	if( !circular_buf_get(rxRingBuff, &data))
		return data;
	
	return  0;	
}

/*
@description -
	USART RX complete ISR vector
@param -
	void
@return -
	void
*/
ISR(USART_RX_vect){
	uint8_t ch;
	
	/* TODO -- ERROR check */
	
	
	/*push to ring buffer if space available */
	ch = UDR0;
	//if(!circular_buf_full(rxBuffer))
	circular_buf_put2(rxRingBuff, ch);
	
	/*callback user function*/
	if(rxCallback != NULL)
		rxCallback();
}

/*
@description -
	USART TX complete ISR vector
@param -
	void
@return -
	void
*/
ISR(USART_TX_vect){
	uint8_t ch;
	
	/* TODO -- ERROR check */

	/* put next data to be transmitted to the buffer if data available */
	if(!circular_buf_get(txRingBuff, &ch))
		UDR0 = ch;
	
	/*callback user function*/
	if(txCallback != NULL)
		txCallback();
}

/*
@description -
	USART TX complete ISR vector
@param -
	void
@return -
	void
*/
ISR(USART_UDRE_vect){
	uint8_t ch;
	
	/* disable this interrupt if there are no data in the buffer */
	if(circular_buf_empty(txRingBuff)){
		UCSR0B &= ~_BV(UDRIE0);
	}else{
		circular_buf_get(txRingBuff, &ch);
		UDR0 = ch; 
	}
}
