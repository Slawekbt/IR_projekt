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

void PWM_enable(void)
{

	TCCR1B |= (1<<CS10) | (1<<WGM12) ; // no pre scaler, CTC mode
	TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B); // OUTPUT  compare IRQ

	/* OCR1A set as equivalent for 38kHz */
	OCR1A = 210; // OCR1A = F_CPU / ( f_PWM * 2 * N - 1) -1 , where N == 1, because no precsaler
	OCR1B = 70; // duty cycle == 1/4

}

void PWM_disable(void)
{
	TCCR1B &= ~(1<<CS10);
	TCCR1B &= ~(1<<WGM12);
	TIMSK1 &= ~(1<<OCIE1A);
	TIMSK1 &= ~(1<<OCIE1B);
}

void samsung_transmission_begin(unsigned char *raw_signal) // raw_signal
{

	unsigned char i;
    PWM_enable();

	for(i=0; i < 120 ; i++)
	{
		if( !(raw_signal[i]) ) // in raw_signal '0' and '1' are inverted, thus !(). If '1' detected send PWM signal
		{
			sei();              // enable PWM timer1 interrupt, for physical '1' 38kHz modulated signal, 1/3 duty cycle
			_delay_us(562.5);
		}

		else
		{
			cli();                // disable PWM timer1 interrupt, for physical '0' no signal from IR diode
			PORTD &= ~(1<PORTD3); // set output to '0', LED_OFF
			_delay_us(562.5);
		}


	}

	PWM_disable();

}

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
			_delay_us(282);    // wait 562.5 / 2 to get 'in the middle' of i-signal
			raw_signal[i] = ( (PIND & (1<<PIND2))); // save to raw_signal PIND2 value 0x00 or 0x04;
			_delay_us(282.5);  // wait until end of i-signal
		}

		for (i=0; i<120; i++)		  // Send to terminal, divide by 0x04 to get 0's and 1's
		{
			raw_signal[i] = raw_signal[i]  / 0x04; // PIND2 value, divide by 0x04 to get one's and zero's, PIND.2 HIGH state == 0x04
			USART_Transmit(raw_signal[i]);
		}



}



int main(void)
{

	USART_Init(MYUBRR);
    EIMSK |= (1<<INT0); 	// INT0 ENABLE
	EICRA = (0<<ISC00) | (0<<ISC01); // LOW LEVEL
	EIFR |= (0<<INTF0); // CLEAR INTERRUPT FLAG


	DDRD &=~(1<<DDD2);    // INPUT IR RECEIVER
	PORTD |= (1<<PORTD2); // PULLUP

	DDRD |= (1<<DDD3);    // OUTPUT for "Transmission"


	PORTD |= (1<<PORTD7); //INPUT  Button_3 :: IR transmission of last received raw signal
	DDRD &=~(1<<DDD7);

	PORTD |= (1<<PORTD5); //INPUT  Button_2 :: protocol check, command parity check
	DDRD &=~(1<<DDD5);

	PORTD |= (1<<PORTD4); // INPUT, Button_1 :: decode, print decoded_short, print code_short [HEX]
	DDRD &=~(1<<DDD4);
	sei();


    while (1)
    {


		/* Button_0 */

		if( !(PIND & (1<<PIND4)) ) // Terminal in HEX mode, numbers only
		{
			_delay_ms(300); // debounce
			raw_decoder(raw_signal,decoded_signal); // decode
			unsigned char protcol_id = protocol_check(decoded_signal);

			if(protcol_id) // 1 for NEC
			{
				unsigned int i;
				code_short = short_decoder(decoded_signal, decoded_short_nec); // changes global table decoded_short_nec[24] , returns code_short [Hex]
				for(i=0; i<24 ; i++)
				{
					USART_Transmit(decoded_short_nec[i]);
				}

				USART_Send_Short_Code(code_short, decoded_signal);
			}
			else  // 0 for SAMSUNG
			{
				unsigned int i;
				code_short = short_decoder(decoded_signal, decoded_short_samsung); // changes global table decoded_short_samsung[32] , returns code_short [Hex]
				for(i=0; i<32 ; i++)
				{
					USART_Transmit(decoded_short_samsung[i]);
				}

				USART_Send_Short_Code(code_short, decoded_signal);
			}

		}

		/* Button_1 */

		if( !(PIND & (1<<PIND5)) ) // Terminal in char mode, char String[] only, press Button_0 first to decode !!!
		{

			/* Parity check */
			_delay_ms(300); // debounce
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

		/* Button_2 */

		if( !(PIND & (1<<PIND7)) ) // Try IR_Transmit

		{
			_delay_ms(300); // debounce
			samsung_transmission_begin(raw_signal);
			sei();
		}


	}
}


