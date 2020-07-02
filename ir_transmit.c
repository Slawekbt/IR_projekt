#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ir_receive.h"
#include "ir_display.h"
#include "ir_transmit.h"


void PWM_enable(void)
{

	TCCR1B |= (1<<CS10) | (1<<WGM12) ; // no pre scaler, CTC mode
	TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B); // OUTPUT  compare IRQ

	/* OCR1A set as equivalent for 38kHz */
	OCR1A = 210; // OCR1A = F_CPU / ( f_PWM * 2 * N - 1) -1 , where N == 1, because no precsaler
	OCR1B = 94; // duty cycle == 1/3  || 8.8 us ON, 17.6us OFF according to Samsung datasheet

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
		if( !(raw_signal[i]) )  // in raw_signal '0' and '1' are inverted, thus !(). If '1' detected send PWM signal
		{
			sei();              // enable PWM timer1 interrupt, for physical '1' 38kHz modulated signal, 1/3 duty cycle
			_delay_us(560);
		}

		else
		{
			cli();                // disable PWM timer1 interrupt, for physical '0' no signal from IR diode
			PORTD &= ~(1<PORTD3); // set output to '0', LED_OFF
			_delay_us(560);
		}


	}

	PWM_disable();

}


void Choose_instance(unsigned char *inst_counter) // PINDn attached, inst_counter globally defined in main.c
{
		
	if(*(inst_counter) < 11) // there is 10 instances
	{
		*(inst_counter) += 1; 
	}
	else
	{
		*(inst_counter) = 1 ; // return back to Instance_1
	}
	
}


void Code_assigner(unsigned char choose_inst, unsigned char *raw_table, long int code_short) // save received code to Instance based on inst_couter
{
	unsigned char save_protocol_id = protocol_check(raw_table); // 0 for SAMSUNG, 1 for NEC
	unsigned char i;
	
	switch (choose_inst)
	{
		case Button_1:
		USART_putString( "  ### Instance_1 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{	
			
			Inst_1.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_1.samsung_send_raw[i] = raw_table[i];

			}
			
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_1.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_2:
		USART_putString( "  ### Instance_2 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{	
			
			Inst_2.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_2.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_2.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_3:
		USART_putString( "  ### Instance_3 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_3.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_3.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_3.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_4:
		USART_putString( "  ### Instance_4 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_4.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_4.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_4.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_5:
		USART_putString( "  ### Instance_5 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_5.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_5.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_5.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_6:
		USART_putString( "  ### Instance_6 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_6.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_6.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_6.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_7:
		USART_putString( "  ### Instance_7 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_7.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_7.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_7.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_8:
		USART_putString( "  ### Instance_8 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_8.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_8.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_8.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_9:
		USART_putString( "  ### Instance_9 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_9.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_9.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_9.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;
		case Button_0:
		USART_putString( "  ### Instance_0 ###  " );
		if(save_protocol_id == 0) // if SAMSUNG code received, attach its short_code [HEX] and whole raw_signal table to instance
		{
			Inst_0.samsung_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // SAMSUNG short_code to terminal
			
			for(i=0 ; i<120 ; i++)
			{
				Inst_0.samsung_send_raw[i] = raw_table[i];
			}
			
		}
		else // if NEC code received, attach its short_code [HEX] as identifier, then, if NEC_short_code detected, transmission of samsung_send_raw starts
		{
			Inst_0.nec_short_code = code_short;
			USART_Send_Short_Code(code_short, raw_table); // send NEC short_code to terminal
		}
		break;

	}
	
}

void Inst_transmit(long int code_short) // code short received, assigned NEC-SAMSUNG transmission mode
{
	if(code_short == Inst_1.nec_short_code)
	{
		samsung_transmission_begin(Inst_1.samsung_send_raw);
	}
	else if(code_short == Inst_2.nec_short_code)
	{
		samsung_transmission_begin(Inst_2.samsung_send_raw);
	}
	else if(code_short == Inst_3.nec_short_code)
	{
		samsung_transmission_begin(Inst_3.samsung_send_raw);
	}
	else if(code_short == Inst_4.nec_short_code)
	{
		samsung_transmission_begin(Inst_4.samsung_send_raw);
	}
	else if(code_short == Inst_5.nec_short_code)
	{
		samsung_transmission_begin(Inst_5.samsung_send_raw);
	}
	else if(code_short == Inst_6.nec_short_code)
	{
		samsung_transmission_begin(Inst_6.samsung_send_raw);
	}
	else if(code_short == Inst_7.nec_short_code)
	{
		samsung_transmission_begin(Inst_7.samsung_send_raw);
	}
	else if(code_short == Inst_8.nec_short_code)
	{
		samsung_transmission_begin(Inst_8.samsung_send_raw);
	}
	else if(code_short == Inst_9.nec_short_code)
	{
		samsung_transmission_begin(Inst_9.samsung_send_raw);
	}
	else if(code_short == Inst_0.nec_short_code)
	{
		samsung_transmission_begin(Inst_0.samsung_send_raw);
	}
}
