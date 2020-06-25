#ifndef _IR_DISPLAY_H
#define _IR_DISPLAY_H 1

void USART_Init(unsigned int); // UBRR
unsigned char USART_Receive(void);
void USART_Transmit(unsigned char); // data
void USART_putString(char* ); // char String[]
void USART_Send_Short_Code(long int, unsigned char *); // code short, decoded_signal [] ,showcases HEX code of signal


#endif /* stdio.h */