#include "ir_receive.h"
#include <avr/io.h>

/* First level decoder

detect start bit
detect coding sequence

decode received signal to logicals:
- logical 0 = 01
- logical 1 = 0111

Received IR signal bits are inverted
For transmission issues every bit must be flipped in 'raw_signal'

Decoded signal sequence includes

| 16 zeros start bit| 8 ones start bit| 8 bit address | 8 bit inverted address| 8 bit command | 8 bit inverted command| if NEC protocol used
| 8 zeros start bit | 8 ones start bit| 8 bit address | 8 bit repeated address| 8 bit command | 8 bit inverted command| if SAMSUNG protocol used

 */



/* Detect number of 0 start bits */

unsigned char start_bit_0_decoder(unsigned char *raw_signal)  // find number of '0's in start_bit sequence
{
	unsigned char i;
	unsigned char counter_bit_0 = 0;
	for(i = 0; i < 18 ; i++)                                  // for NEC 16 0's, for SAMSUNG 8 0's
	{
		if(raw_signal[i]==0)
		{
			counter_bit_0 += 1;                               // counter bit indicated number of '0's 
		}
		else
		{
			break ;                                           // detect end of 0's start bit sequence
		}
	}
															  // counter_bit_0 == number of '0' start bits in sequence
	return counter_bit_0;                                     // raw_signal [counter_bit_0] == start of next bit sequence 
}


/* Detect number of 1 start bits */

unsigned char start_bit_1_decoder(unsigned char *raw_signal)
{
	unsigned char counter_bit_1 = start_bit_0_decoder(raw_signal);
	unsigned char i;

	for (i = counter_bit_1 ; i < counter_bit_1 + 18 ; i ++) // start from i = counter_bit_1 == counter_bit_0, place, where 0's sequence ended
	{
		if(raw_signal[i] == 1)
		{
			counter_bit_1 +=1;
		}

		else
		{
			break ;                                         // detect end of '1's bit start sequence
		}
	}
															// counter_bit_1 == number of '1' start bits in sequence
	return counter_bit_1;									// for return counter_bit_1,  raw_signal[counter_bit_1] -> next bit sequence
	
}


unsigned char protocol_check(unsigned char *decoded_signal)
{
	unsigned char identifier = 0;
	unsigned char start_bit_0 = start_bit_0_decoder(decoded_signal);
	
	if(start_bit_0 == 8)
	{
		identifier = 0; // 0 for SAMSUNG PROTOCOL
	}
	
	else if(start_bit_0 == 16)
	{
		identifier = 1;  // 1 for NEC PROTOCOL
	}
	
	return identifier ;
	
}


void raw_decoder(unsigned char *raw_signal, unsigned char *decoded_signal)                 // decode whole signal, including start bits
{
	unsigned char start_bit_0 = start_bit_0_decoder(raw_signal);
	unsigned char start_bit_1 = start_bit_1_decoder(raw_signal);

	unsigned char i;


	for(i=0; i< start_bit_0; i++)
		{
			decoded_signal[i] = 0;
		}

	for(i= start_bit_0; i< start_bit_1; i++)
		{
			decoded_signal[i] = 1;
		}


	unsigned char j = start_bit_1;
	unsigned char flag = 0;

	for(i = start_bit_1 ; i < 120 ; i++ )
	{
		flag  = ( 1 ^ raw_signal[i] ) & raw_signal[i+1] & raw_signal[i+2] & raw_signal[i+3] ;  // detect logical '1' or '0'
		decoded_signal[j] = flag ;

		if(flag == 1) // if logical '1' detected, move by four in raw_signal
		{
			i+=3;
		}

		else
		{
			i+=1;   // if logical '0' detected, move by two in raw_signal
		}

		j+=1;       // move to next element in decoded_signal
	}


}

/*

Second level decoding process

remove 0's start bits
remove surplus 0's and the end of decoded table
output presented in HEX


 */

/* *decoded_short_ is not returned by short_decoder()
	decoded_short_ must be:
	- 24 bit length for NEC
	- 32 bit length for SAMSUNG
	
	globally initialized decoded_short_nec[24] or decoded_short_samsung[32] must be provided
	additional if-statement needed in main.c to find out, which table should be used ::  protocol_check();
	
	short_decoder returns long int 'short_value_' that is binary equivalent of whole decoded_short_ signal table 

 */

long int short_decoder(unsigned char *decoded_signal, unsigned char *decoded_short_)				// long int is needed for 32 bit data representation - SAMSUNG, and 24 bit - NEC
{	
	long int short_value_ = 0x00000000;
	unsigned char protocol_id = protocol_check(decoded_signal);
	
	if(protocol_id) // NEC short
	{
		unsigned char i;
		
		for(i=0; i<8 ; i ++)        // 8-bit start bit '1' NEED's A CHANGE
		{
			decoded_short_[i] = 1; // 8-bit one's start BIT representative 
		}
		
		for(i=8; i<24 ; i++)         // 16 bits of message
		{
			decoded_short_[i] = decoded_signal[i+32]; // message_bit starts at 40 position counting from 0, here i = 8, so decoded_signal[8+32] == first message_bit
		}
		
		/* HEX representation
		   turn table of values into one binary value, using right shift
		   binary value can be then showcased in terminal in HEX representation
		   just for Terminal issues
		 */
		
		for(i=0; i<24 ; i++) // check this
		{	
			if(decoded_short_[i]) // if set 1
			{
				short_value_ |= (1 << (23 - i) );  // 23-i, first element is the MSB, last element in decoded_short_ is LSB, that works well
			}
			else // if set 0
			{
				short_value_ &= ~(1 << (23 - i) );  // 23-i, first element is the MSB, last element in decoded_short_ is LSB, that works well
			}
		}
		
		
	}
	else if (!(protocol_id )) // SAMSUNG short
	
	{
		unsigned char i;
		for(i=0 ; i < 32 ; i++)
		{
			decoded_short_[i] = decoded_signal[i+16] ;  // omit | 8 zeros start bit | 8 ones start bit| sequence
		}
		
		for(i=0 ; i < 32 ; i++)
		{
			short_value_ |= ( decoded_short_[i] << (31 - i) );
		}
		
		
	}
	return short_value_; // return 24 or 32 binary value of decoded signal, problems here
}

unsigned char parity_check(long int decoded_short_) // decoded_short_nec or decoded_short_samsung must be provided, same procedure for both
{	
	unsigned char flag = 0; 
	unsigned char _signal;
	unsigned char inv_signal;
	
	_signal = (decoded_short_>>8) & 0xff;
	inv_signal = decoded_short_ & 0xff;
	
	flag = ~(_signal ^ inv_signal);  // if 0x00 parity check succeeded, else an Error during receiving must have occurred 
	
	return flag;
}