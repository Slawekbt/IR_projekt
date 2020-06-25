#ifndef _IR_RECEIVE_H
#define _IR_RECEIVE_H 1

unsigned char start_bit_0_decoder(unsigned char * ); // unsigned char table[] input
unsigned char start_bit_1_decoder(unsigned char * ); // unsigned char table[] input

unsigned char protocol_check(unsigned char *); // decoded_signal (or raw_signal) must be provided , returns 0 for SAMSUNG, 1 for NEC

void raw_decoder(unsigned char *, unsigned char *);   // decode whole raw_signal, including start bits
long int short_decoder( unsigned char *, unsigned char *); // provide decoded_signal, decoded_short_signal (NEC [24] or SAMSUNG [32] )

unsigned char parity_check(long int); // provide binary output out of short_decoder(), 0 for SUCCESS, else ERROR


#endif /* stdio.h */