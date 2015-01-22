#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H

#include "inttypes.h"

#define TX_PIN		PB0
#define TX_HI		PORTB |= (1<<TX_PIN)
#define TX_LOW		PORTB &= ~(1<<TX_PIN)
#define CLF_CMPA 	TIFR0|=(1<<OCF0A)
#define CLF_PC1		GIFR|=(1<<PCIF1)

void SerialBegin(void);

/*--------------------------------------------------------------*/
/*	SERIAL WRITE												*/
/*	Writes on byte to the USART port							*/
/*--------------------------------------------------------------*/
void SerialWrite(uint8_t write_byte);

void enableTimer(void);

void disableTimer(void);

#endif
