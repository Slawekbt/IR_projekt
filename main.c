/*
 * IR main

 * Author : S³awomir Tenerowicz
 */

#define F_CPU 16000000 // 16 MHz must be here, otherwise -- redefined error, all delay utilities does not work !

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ir_receive.h"
#include "ir_display.h"
#include "ir_transmit.h"

#define FCOS 16000000
#define BAUD 9600
#define MYUBRR FCOS/16/BAUD-1

/* Transmit utilities */

#define LED_ON PORTD |=(1<<PORTD3)
#define LED_OFF PORTD &= ~(1<<PORTD3)

ISR(TIMER1_COMPA_vect) // gap time 0 : OCR1B
{
	LED_ON;
}

ISR(TIMER1_COMPB_vect) // gap time OCR1B : OCR1A
{
	LED_OFF;
}

unsigned char inst_counter = 1; // default Inst_1 

/* End of Transmit utilities */

/* global decoding variables */
unsigned char raw_signal[120] = {};     // table for raw input IR data
unsigned char decoded_signal[64] = {};  // table for 1st_step decoded data, 64 bit for NEC, 48 bit for SAMSUNG, raw_decoder() output
long int code_short = 0x00 ;               // NEC 24-bit or SAMSUNG 32-bit code saved as one number

/* NEC global variables */
unsigned char decoded_short_nec[24] ={}; // table for NEC 24-bit code, input for short_decoder();

/* SAMSUNG global variables */
unsigned char decoded_short_samsung[32] ={}; // table for SAMSUNG 32-bit code, input for short_decoder();



ISR(INT0_vect)                         // ! received IR bits are inverted !
{
	    cli();  // do not interrupt receiving process
		unsigned int i ;
		for (i=0; i<120; i++)
		{
			_delay_us(282);    // wait  ( 562.5 / 2 ) us to get 'in the middle' of i-signal value
			raw_signal[i] = ( (PIND & (1<<PIND2))); // save to raw_signal PIND2 value 0x00 or 0x04;
			_delay_us(282.5);  // wait until end of i-signal
		}

		for (i=0; i<120; i++)		  //  divide by 0x04 to get 0's and 1's
		{
			raw_signal[i] = raw_signal[i]  / 0x04; // PIND2 value, divide by 0x04 to get one's and zero's, PIND.2 HIGH state == 0x04;
		    USART_Transmit(raw_signal[i]);         // BUG here : IF no TRANSMIT wrong output provided
		}

		/* For instant NEC-SAMSUNG transmission sake
		   it must immediately return code_short for identification
		 */
		
		raw_decoder(raw_signal, decoded_signal);
		unsigned char protocol_id = protocol_check(decoded_signal);

			if(protocol_id) // 1 for NEC
			{
				code_short = short_decoder(decoded_signal, decoded_short_nec); // changes global table decoded_short_nec[24] , returns code_short [Hex]
			}
			else  // 0 for SAMSUNG
			{
				code_short = short_decoder(decoded_signal, decoded_short_samsung); // changes global table decoded_short_samsung[32] , returns code_short [Hex]
			}
		USART_Send_Short_Code(code_short, decoded_signal);

}



int main(void)
{

	USART_Init(MYUBRR); // Terminal transmission initializer 9600 baud
	
    EIMSK |= (1<<INT0); 	         // INT0 ENABLE
	EICRA = (0<<ISC00) | (0<<ISC01); // LOW LEVEL
	EIFR |= (0<<INTF0);              // CLEAR INTERRUPT FLAG


	DDRD &=~(1<<DDD2);    // INPUT IR RECEIVER attached (INT0)
	PORTD |= (1<<PORTD2); // PULLUP

	DDRD |= (1<<DDD3);    // PWM OUTPUT for "Transmission"
	

	PORTB |= (1<<PORTB0); // INPUT,  External_Button_5 ::  Assigned NEC-SAMSUNG Transmission
	DDRB &=~(1<<DDB0);	
	
	PORTD |= (1<<PORTD7); // INPUT,  External_Button_4 :: Do NEC-SAMSUNG code assignment
	DDRD &=~(1<<DDD7);
	
	DDRD &= ~(1<<DDD6);   // INPUT,  External_Button_3 :: Choose instance Button
	PORTD |= (1<<PORTD6);

	PORTD |= (1<<PORTD5); // INPUT,  External_Button_2 :: protocol check, command parity check
	DDRD &=~(1<<DDD5);

	PORTD |= (1<<PORTD4); // INPUT,  External_Button_1 :: print decoded_short, print code_short [HEX]
	DDRD &=~(1<<DDD4);
	

	sei();


    while (1)
    {


		/* External_Button_1 */

		if( !(PIND & (1<<PIND4)) ) // Terminal in HEX mode, numbers only
		{
			_delay_ms(300);
			/* 
			   
			   Signal decoded and code_short created at INT0
			
			   raw_signal --> decoded_signal
			   decoded_signal --> code_short
			   
			 */
			unsigned char protocol_id = protocol_check(decoded_signal);

			if(protocol_id) // 1 for NEC
			{
				unsigned int i;
				for(i=0; i<24 ; i++)
				{
					USART_Transmit(decoded_short_nec[i]);
				}

				USART_Send_Short_Code(code_short, decoded_signal);
			}
			else  // 0 for SAMSUNG
			{
				unsigned int i;
				for(i=0; i<32 ; i++)
				{
					USART_Transmit(decoded_short_samsung[i]);
				}

				USART_Send_Short_Code(code_short, decoded_signal);
			}

		}

		/* External_Button_2 */

		if( !(PIND & (1<<PIND5)) ) // Terminal in char mode
		{

			/* Parity check */
			_delay_ms(300);
			unsigned char parity_id = parity_check(code_short);
			if(!(parity_id))
			{
				USART_putString("Command parity check SUCCEEDED ");
			}
			else
			{
				USART_putString("Command parity check FAILED ");
			}

			/* Protocol check */

			unsigned char protocol_id = protocol_check(decoded_signal);
			if(protocol_id == 0)
			{
				USART_putString(" SAMSUNG IR Code Detected ");
			}
			else
			{
				USART_putString(" NEC IR Code Detected ");

			}


		}

		/* External_Button_3 */

		if( !(PIND & (1<<PIND6)) ) // Choose instance

		{	

			Choose_instance(&inst_counter); // send inst_counter_ADD to function
			USART_Transmit(inst_counter); // Check current save-position
			_delay_ms(600);
		}
		
		/* External_Button_4 */
		
		if( !(PIND & (1<<PIND7)) ) // NEC-SAMSUNG code assignment to Instance_('inst_counter')

		{
			_delay_ms(500);
			Code_assigner(inst_counter, raw_signal, code_short);
			
		}
		
		/* External_Button_5 */
		
		if(	!(PINB & (1<<PINB0)) ) // instant NEC-SAMSUNG transmission mode
		
		{
			code_short = 0x00 ; // reset code_short value
			USART_putString("  ### Instant NEC-SAMSUNG transmission mode ###  ");

			while(1)
			{
				sei();         // prevent clutching in cli()
				if(code_short) // if IR signal received (and decoded)s
				{
					Inst_transmit(code_short); // do transmit according to code_short value ( choose appropriate Instance, if code_short assigned )
					_delay_ms(50);
					code_short = 0x00; // reset code_short value
				}
				
				else if(!(PIND & (1<<PIND5))) // use External_Button_2 to break the loop and end the 'Instant transmission mode'
				{	
					USART_putString("  ### Instant NEC-SAMSUNG transmission mode has been ended ###  ");
					break; 
				}
				
			} 
		}	
		

	}
}


