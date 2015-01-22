/*
A Serial Transmitter used for debugging!

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "SerialDebug.h"
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t TTRIG = 0;
volatile uint8_t DATA_STATE = 1;

void SerialBegin() {
	DDRB |= (1<<TX_PIN);// OUTPUTS
	sei();// enable global interrupts
	_delay_ms(100);
	
	/* SETUP COUNTER */
	TCCR0A	|= 0x02;
	TCCR0B	|= 0x02; // x1 prescaler
	OCR0A	= F_CPU/8/19200; // set timer compare A for period of 1bit at 19200baud
}// SerialBegin()

void SerialWrite(uint8_t write_byte){
	uint8_t i;
	TX_HI;
	
	enableTimer();
	TTRIG = 0;
	DATA_STATE = 0;// start bit
	while(!TTRIG);// wait for interrupt to transmit start bit
	for(i=0; i<8; i++){
		TTRIG = 0;
		DATA_STATE = (write_byte>>i)&0x1;
		while(!TTRIG);// wait for data bit to send;
	}// for
	/* STOP BITS */
	TTRIG = 0;
	DATA_STATE = 1;
	while(!TTRIG);
	/* 2 IDLE BITS */
	TTRIG = 0;
	while(!TTRIG);
	TTRIG = 0;
	while(!TTRIG);
	disableTimer();
}

void enableTimer(){
	/* reset the counter register to zero */
	TCNT0	= 0x0;
	CLF_CMPA;// clear flag
	TTRIG = 0;
	sei();
	TIMSK0 |= (1<<OCIE0A);// enable timer interrupt
}// enableTimer()

void disableTimer(){
	TIMSK0 &= ~(1<<OCIE0A);// disable timer interrupt
}// disableTimer()

ISR(TIM0_COMPA_vect){
	if(DATA_STATE)	TX_HI;
	else			TX_LOW;
	TTRIG = 1;
	return;
}// ISR(TIM1_COMPA_vect)
