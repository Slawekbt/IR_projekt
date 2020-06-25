#include <avr/io.h> 
#include "ir_display.h"
#include "ir_receive.h"

void USART_Init(unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (3<<UCSZ00);
}


unsigned char USART_Receive(void)
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)))
	;
	/* Get and return received data from buffer */
	return UDR0;
}

void USART_Transmit(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)))
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

void USART_putString(char* StringPtr)      // Define String as pointer to StringPtr - it's table in fact
{
	while(*StringPtr != 0x00)          // Each string table ends with EOL == 0x00
	{
		USART_Transmit(*StringPtr);    // Print single char of StringPtr
		StringPtr++;				   // Increment pointer, get next char value
	}
}

void USART_Send_Short_Code(long int code_short, unsigned char *decoded_signal)
{
	/*
		code_short_NEC      00 FF FF FF
		code_short SAMSUNG  FF FF FF FF
	
	*/
	
	unsigned char protocol_id = protocol_check(decoded_signal);
	
	if( !(protocol_id) ) // For SAMSUNG, 4 x 8 bit
	{
		
		unsigned char i;
		unsigned char samsung_code_display[4] = {};
		
		for(i=0 ; i<4 ; i++)
		{
			samsung_code_display[i] = (code_short >> (8*i) ) & 0xff; // divide long int into 8-bit four pieces, reversed
		}
		
		for(i=0 ; i<4 ; i++)
		{
			USART_Transmit(samsung_code_display[3-i]); // display backwards, to get original code
		}
		
	}
	
	else // for NEC 3 x 8 bit
	{
		unsigned char i;
		unsigned char nec_code_display[3] = {0x00,0x00,0x00};
		
		for(i=0 ; i<3 ; i++)
		{
			nec_code_display[i] = ( (code_short >> (8*i) ) & 0xff ) ; // divide long int into 8-bit three pieces, reversed
		}
		
		for(i=0 ; i<3 ; i++)
		{	
			USART_Transmit(nec_code_display[2-i]); // display backwards, to get original code
		}		
	}
	

	
}


