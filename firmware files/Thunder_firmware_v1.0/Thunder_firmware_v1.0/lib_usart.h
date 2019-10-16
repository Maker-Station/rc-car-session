/*
 * lib_usart.h
 *
 * Created: 10/12/2019 7:24:53 PM
 *  Author: workstation
 */ 


#ifndef LIB_USART_H_
#define LIB_USART_H_


#define USART_DEFAULT_BUFFER_SIZE	64
#define USART_RX_BUFFER_SIZE		USART_DEFAULT_BUFFER_SIZE
#define USART_TX_BUFFER_SIZE		USART_DEFAULT_BUFFER_SIZE




/*
@description -
	Defines the supported serial communication modes by the library
*/
enum usart_mode_t{
	USART_ASYNCH_MODE = 0x00,
	USART_SYNCH_MODE = 0x40,
	USART_MASTER_SPI_MODE = 0xc0
};

/*
@description -
	Defines the supported parity modes USART
*/
enum usart_parity_mode_t{
	USART_PARITY_DISABLE = 0x00,
	USART_PARITY_ODD = 	0x20,
	USART_PARITY_EVEN = 0x30
};

/*
@description -
	Defines the supported stop bits 
*/
enum usart_stop_bit_t{
	USART_STOPBITS_1x = 0x00,
	USART_STOPBITS_2x = 0x08,
};

/*
@description -
	Defines the supported data frame sizes
*/
enum usart_dataFrame_size_t{
	USART_DATA_FRAME_5x = 0x00,
	USART_DATA_FRAME_6x = 0x02,
	USART_DATA_FRAME_7x = 0x04,
	USART_DATA_FRAME_8x = 0x06,
	USART_DATA_FRAME_9x = 0x07,
	USART_DATA_FRAME_t_cnt = 5
};

/*
@description -
	USART interrupt callback function type
*/
typedef void (*usart_callback_t)(void);

/*
*/
struct usart_config_t{
	unsigned long boudrate;
	enum usart_mode_t mode;
	enum usart_dataFrame_size_t dFrameSize;
	enum usart_parity_mode_t parityMode;
	enum usart_stop_bit_t stopBits;
	
	bool synchModeEdge;
	
	uint8_t txBuffer_size, rxBuffer_size;
	
	usart_callback_t rxCallback;
	usart_callback_t txCallback;
};


/************** LIB_USART function definitions **********************/

void lib_usart_get_defaults(struct usart_config_t *cfg);		//load default values to the config structure
int lib_usart_init(struct usart_config_t *cfg);					//set usart hardware according to the structure

void lib_uart_enable();					// enable the serial port
void lib_uart_disable();				// disable the serial port
void lib_uart_enable_interrupts();		// enable the serial port interrupts 
void lib_uart_disable_interrupts();		// disable the serial port interrupts

bool lib_usart_available();				// returns whether data is available or not to read
char lib_usart_read();					// read character received to serial port
int lib_usart_write_byte(uint8_t data);				// write byte to serial port
int lib_usart_write(uint8_t* data, uint8_t len);	// write byte array to serial port



int lib_usart_write_char(char data);	//write one byte to serial port
int lib_usart_write_string(char str[]);
int lib_usart_read_char(char *ch);


#endif /* LIB_USART_H_ */