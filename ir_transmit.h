#ifndef _IR_TRANSMIT_H
#define _IR_TRANSMIT_H 1

struct IR_sending_unit_ // data type to store NEC-SAMSUNG assigned transmission utilities
{
	long int nec_short_code ;      // nec_short_code as ID for samsung_raw_send[]
	long int samsung_short_code ;  // for check purpose, may be deleted -- to much memory space occupied
	unsigned char samsung_send_raw[120] ; // actual SAMSUNG raw signal to send
};

typedef struct IR_sending_unit_ IR_sending_unit;

/* Enumerate Buttons on NEC remote */


typedef enum { 
	Button_1 = 1,
	Button_2,
	Button_3,
	Button_4,
	Button_5,
	Button_6,
	Button_7,
	Button_8,
	Button_9,
	Button_0,
 } NEC_Buttons_Inst;

IR_sending_unit Inst_1, Inst_2, Inst_3, Inst_4, Inst_5, Inst_6, Inst_7, Inst_8, Inst_9, Inst_0; // define Global instances, each instance
																							    // represents another button on NEC remote board and
																								// its equivalent assigned SAMSUNG code


void samsung_transmission_begin(unsigned char *); // raw_signal, send raw_signal 38kHz modulated
void Choose_instance(unsigned char *); // choose between Inst_1, Inst_2, .. appropriate signal will be saved
void Code_assigner(unsigned char ,unsigned char *, long int ); // depending on enum choose_inst, and type NEC/SAMSUNG assign code to choose_inst instance
void Inst_transmit(long int); // depending on NEC code_short received, transmit assigned raw_signal_samsung in Instance

#endif /* stdio.h */