#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "hd44780.h"
#include "main.h"
#include "temp_lut.h"
#include "SerialDebug.h"

uint8_t j;
int8_t calibration_offset = 0xF0;
uint16_t max_batt = 34;
uint16_t eeprom_used = 3;
uint32_t temperature;
uint16_t sample[12];
uint8_t used_slots = 0;
uint8_t crnt_slot = 0;
uint8_t crnt_menu = MAIN_MENU;
struct menu_ptr selector = {0,0,2,1,MAIN_MENU};
static FILE lcd_stdout = FDEV_SETUP_STREAM(lcd_printf, NULL, _FDEV_SETUP_WRITE);
static FILE ser_stdout = FDEV_SETUP_STREAM(ser_printf, NULL, _FDEV_SETUP_WRITE);

/***************************** MAIN ****************************/
int main(void) {
	//****SETUP****
	eeprom_used+=eeprom_read_word((uint16_t*)0)+3;
	if(eeprom_used%2){ // clear eeprom if value is not word aligned
		eeprom_write_word((uint16_t*)0,0);
		eeprom_used=3;
	}// if
	DDRA&=~((1<<SEL_PIN)|(1<<UP_PIN)|(1<<DOWN_PIN)); // INPUTS
	ADMUX=(0x00);					// select ADC0; voltage reference to VCC
	ADCSRA|=(1<<ADPS2|1<<ADPS1|1<<ADPS0);	// set prescalor to 128 (ADC clock running a 8MHz/128=62.5KHz)
	ADCSRA|=(1<<ADEN);				// enable ADC0
	DIDR0|=(1<<ADC0D);				// disable digital input PA0
	stdout = &lcd_stdout;
	SerialBegin();
	lcd_init();
	lcd_clrscr();
	lcd_command(_BV(LCD_DISPLAYMODE) | _BV(LCD_DISPLAYMODE_ON));
	change_menu();

	//****LOOP****
	while(1){
		if(READ_SEL) {
			if((crnt_menu==SINGLE_MENU)&&(selector.next_menu==SINGLE_MENU)){
				/* RECORD SAMPLE */
				if(j>=11){
					selector.next_menu=SAVE_MENU;
					change_menu();
				} else {
					j++;
				}// if
			} else if(crnt_menu==SAVE_WHERE){
				/* SAVE SAMPLE */
				for(j=0;j<12;j++){
					eeprom_write_word((uint16_t*)((crnt_slot*24)+(j*2)+3),sample[j]);
				}// for
				eeprom_used+=24;
				eeprom_write_word((uint16_t*)0,eeprom_used-2); // minus 2 because of the 2 bytes to store 'eeprom_used' value
				selector.next_menu=RECORD_MENU;
				change_menu();
			} else if((crnt_menu==MEMORY_MENU)&&(selector.next_menu==MEM_CLR_MODE)){
				/* CLEAR MEMORY */
				eeprom_write_word((uint16_t*)0,0);
				eeprom_used=2;
			} else if((crnt_menu==LOAD_WHERE)){
				selector.next_menu=RECALL_MODE;
				change_menu();
			} else {
				change_menu();
			}// if
			while(READ_SEL);
		} else if(READ_UP) {
			move_ptr(&selector,UP);
			display_menu(crnt_menu);
			while(READ_UP);
		} else if(READ_DOWN) {
			move_ptr(&selector,DOWN);
			display_menu(crnt_menu);
			while(READ_DOWN);
		}// if
		
		_delay_ms(75);	// debounce/UI refresh rate
		mode_handler();
		
	}// while00
return 0;
}

uint32_t adc2f(uint16_t adc){
	/* READ VALUES FROM LOOKUP TABLE */
	uint32_t seg_slope = pgm_read_word(&slope[adc>>5]);
	uint32_t seg_y_int = pgm_read_word(&y_int[adc>>5]);
	/* COMPUTER THE TEMPERATURE */
	return (((seg_slope*(uint32_t)adc)>>11)+seg_y_int);
}// adc2f(uint16_t adc)

void mode_handler(){
	uint8_t i;
	used_slots = (eeprom_used-2)/24;

	switch(crnt_menu){
		case BATTERY_MENU:
			lcd_goto(0x00);
			printf("BATTERY LEVEL: %u",(get_batt_lvl()/max_batt)*100);
			break;
		case SINGLE_MENU:
			ADCSRA|=(1<<ADSC);			// start ADC conversion
			while(ADCSRA&(1<<ADSC));	// wait for ADC to complete measurement
			
			lcd_goto(0x00);
			lcd_puts("          ");	// clear the temp
			lcd_goto(0x00);
			sample[j] = ADC;
			temperature = adc2f(sample[j]);
			printf("%u.%0.2u%cF",(uint16_t)(temperature>>4),dp((uint8_t)(temperature&0xF))/100,DEGREE_SYMBOL);
			lcd_goto(0x40);
			lcd_puts_P(&probe_instruction[j]);
			break;
		case MEM_STAT_MODE:
			lcd_goto(0x00);
			printf("%u of 512 bytes", eeprom_read_word((uint16_t*)0)+3);
			break;
		case RECALL_MODE:
			switch(selector.loc){
				case 0: // FRONT
					lcd_clrscr();
					lcd_goto(0x00);
					printf("%3u%3u%3u %3u%3u%3u",(uint16_t)(adc2f(sample[DFO])>>4),(uint16_t)(adc2f(sample[DFC])>>4),(uint16_t)(adc2f(sample[DFI])>>4),(uint16_t)(adc2f(sample[PFI])>>4),(uint16_t)(adc2f(sample[PFC])>>4),(uint16_t)(adc2f(sample[PFO])>>4));
					lcd_goto(0x40);
					lcd_puts_P(&recall_label[0]);
					break;
				case 1: // REAR
					lcd_clrscr();
					lcd_goto(0x40);
					printf("%3u%3u%3u %3u%3u%3u",(uint16_t)(adc2f(sample[DRO])>>4),(uint16_t)(adc2f(sample[DRC])>>4),(uint16_t)(adc2f(sample[DRI])>>4),(uint16_t)(adc2f(sample[PRI])>>4),(uint16_t)(adc2f(sample[PRC])>>4),(uint16_t)(adc2f(sample[PRO])>>4));
					lcd_goto(0x00);
					lcd_puts_P(&recall_label[1]);
					break;
				case 2: // DRIVER SIDE
					lcd_clrscr();
					lcd_goto(0x00);
					printf("%3u%3u%3u",(uint16_t)(adc2f(sample[DFO])>>4),(uint16_t)(adc2f(sample[DFC])>>4),(uint16_t)(adc2f(sample[DFI])>>4));
					lcd_goto(0x40);
					printf("%3u%3u%3u",(uint16_t)(adc2f(sample[DRO])>>4),(uint16_t)(adc2f(sample[DRC])>>4),(uint16_t)(adc2f(sample[DRI])>>4));
					lcd_goto(0x13);
					lcd_putc('F');
					lcd_goto(0x53);
					lcd_putc('R');
					break;
				case 3: // PASSENGER SIDE
					lcd_clrscr();
					lcd_goto(0x0B);
					printf("%3u%3u%3u",(uint16_t)(adc2f(sample[PFO])>>4),(uint16_t)(adc2f(sample[PFC])>>4),(uint16_t)(adc2f(sample[PFI])>>4));
					lcd_goto(0x4B);
					printf("%3u%3u%3u",(uint16_t)(adc2f(sample[PRO])>>4),(uint16_t)(adc2f(sample[PRC])>>4),(uint16_t)(adc2f(sample[PRI])>>4));
					lcd_goto(0x00);
					lcd_putc('F');
					lcd_goto(0x40);
					lcd_putc('R');
					break;
				default:
					break;
			}// switch
			break;
		case LOAD_WHERE:
		case SAVE_WHERE:
			lcd_clrscr();
			lcd_goto(0x00);
			crnt_slot = selector.loc;
			printf("MEMORY SLOT: %c", (int)(crnt_slot+65));
			break;
		case TX_DATA_MODE:
			lcd_clrscr();
			stdout = &ser_stdout;
			printf("%S",&csv_header);
			for(i=0;i<used_slots;i++){
				/* READ FROM EEPROM */
				eeprom_read_block((void*)&sample, (const void*)(i*24)+3, 24);
				printf("%c%c,",'\n',i+65);
				lcd_goto(0x00+i);
				lcd_putc(65+i);
				lcd_goto(0x40);
				lcd_puts_P(&loading_bar);
				/* SEND SAMPLE */
				for(j=0;j<12;j++){
					temperature = adc2f(sample[j]);
					printf("%u.%0.2u,",(uint16_t)(temperature>>4),dp((uint8_t)(temperature&0xF)));
					lcd_goto(0x41+j);
					lcd_putc('=');
					_delay_ms(100);
				}// for
			}// for
			printf("%S",&csv_footer);
			stdout = &lcd_stdout;
			selector.next_menu = MEMORY_MENU;
			change_menu();
			break;
		case INSTANT_MODE:
			ADCSRA|=(1<<ADSC);			// start ADC conversion
			while(ADCSRA&(1<<ADSC));	// wait for ADC to complete measurement
			
			lcd_goto(0x00);
			lcd_puts("          ");	// clear the temp
			lcd_goto(0x00);
			temperature = adc2f((uint16_t)ADC);
			printf("%u.%0.2u%cF",(uint16_t)(temperature>>4),dp((uint8_t)(temperature&0xF))/100,223);
			break;
		case CALIBRATE_MENU:
			ADCSRA|=(1<<ADSC);			// start ADC conversion
			while(ADCSRA&(1<<ADSC));	// wait for ADC to complete measurement
			
			lcd_goto(0x00);
			lcd_puts("          ");	// clear the temp
			lcd_goto(0x00);
			temperature = adc2f((uint16_t)ADC);
			printf("%u.%0.2u%cF",(uint16_t)(temperature>>4),dp((uint8_t)(temperature&0xF))/100,223);
			break;
		default:
			break;
	}// switch
}// mode_handler()

void change_menu(){
	crnt_menu = selector.next_menu;
	switch(crnt_menu){
		case MAIN_MENU:
			selector.loc=0;
			selector.min=0;
			selector.max=2;
			selector.visable=1;
			break;
		case RECORD_MENU:
			j=0;	// reset the sample index
			selector.loc=0;
			selector.min=0;
			selector.max=3;
			selector.visable=1;
			break;
		case SAVE_MENU:
			selector.loc=2;
			selector.min=2;
			selector.max=3;
			selector.visable=1;
			break;
		case SETTINGS_MENU:
			selector.loc=0;
			selector.min=0;
			selector.max=2;
			selector.visable=1;
			break;
		case BATTERY_MENU:
			selector.loc=1;
			selector.min=1;
			selector.max=1;
			selector.visable=1;
			break;
		case SINGLE_MENU:
			selector.loc=2;
			selector.min=2;
			selector.max=3;
			selector.visable=1;
			break;
		case MEMORY_MENU:
			selector.loc=0;
			selector.min=0;
			selector.max=3;
			selector.visable=1;
			break;
		case MEM_STAT_MODE:
			selector.loc=1;
			selector.min=1;
			selector.max=1;
			selector.visable=1;
			break;
		case RECALL_MODE:
			eeprom_read_block((void*)&sample, (const void*)(crnt_slot*24)+3, 24); // read entire sample into ram
			selector.loc=0;
			selector.min=0;
			selector.max=3;
			selector.visable=0;
			break;
		case LOAD_WHERE:
			if(used_slots==0){
				selector.next_menu=MAIN_MENU;
				change_menu();
				break;
			}// if
			selector.loc=0;
			selector.min=0;
			selector.max=used_slots-1;
			selector.visable=0;
			break;
		case SAVE_WHERE:
			selector.loc=used_slots;
			selector.min=0;
			selector.max=used_slots;
			selector.visable=0;
			break;
		case TX_DATA_MODE:
			selector.loc=0;
			selector.min=0;
			selector.max=0;
			selector.visable=0;
			break;
		case INSTANT_MODE:
			selector.loc=1;
			selector.min=1;
			selector.max=1;
			selector.visable=1;
			break;
		case CALIBRATE_MENU:
			selector.loc=2;
			selector.min=2;
			selector.max=2;
			selector.visable=1;
			break;
		default:
			selector.loc=0;
			selector.min=0;
			selector.max=2;
			selector.visable=1;
			crnt_menu=MAIN_MENU;
			break;
	}// switch
	
	move_ptr(&selector,0);
	display_menu(crnt_menu);
}// change_menu()

void move_ptr(struct menu_ptr *ptr, int8_t dir){
	/* UPDATE THE SELECTOR POSITION */
	if((dir==UP)&&(ptr->loc>ptr->min))		ptr->loc+=UP;
	if((dir==DOWN)&&(ptr->loc<ptr->max))	ptr->loc+=DOWN;
	/* UPDATE THE NEXT MENU INDEX */
//	if(ptr->visable)	ptr->next_menu = pgm_read_byte(&menu_jump_table[crnt_menu+ptr->loc]);
	ptr->next_menu = pgm_read_byte(&menu_jump_table[crnt_menu+ptr->loc]);
}// update_ptr(uint8_t dir)

void display_ptr(struct menu_ptr *ptr){
	if(ptr->visable){
		lcd_goto(menu_offset[ptr->loc]-1);
		lcd_putc('>');
	}// if
}// display_ptr(uint8_t p)

void display_menu(uint8_t m){
	uint8_t i;
	
	if(selector.visable){
		lcd_clrscr();
		for(i=0;i<4;i++){
			lcd_goto(menu_offset[i]);
			lcd_puts_P(&menu[m+i][0]);
		}// for
		display_ptr(&selector);
	} else {
		return;
	}// if
}// display_menu(uint8_t m)

uint16_t get_batt_lvl(){
	uint8_t t_admux = ADMUX;
	uint8_t t_adcsra = ADCSRA;
	uint16_t batt;
	ADMUX=(0x21);
	ADCSRA|=(1<<ADPS2|1<<ADPS1|1<<ADPS0|1<<ADEN);
	/* PERFORM A FEW CONVERSIONS TO SETTLE THE CHANNEL CHANGE */
	for(batt=0;batt<5;batt++){
		ADCSRA|=(1<<ADSC);
		while(ADCSRA&(1<<ADSC));
	}// for
	/* MAKE ACTUAL MEASUREMENT */
	ADCSRA|=(1<<ADSC);			// start ADC conversion
	while(ADCSRA&(1<<ADSC));	// wait for ADC to complete measurement
	batt = 11253/ADC;
	/* RESTORE ALL REGISTER */
	ADMUX = t_admux;
	ADCSRA = t_adcsra;
	
	return batt;
}// get_batt_lvl()

uint16_t dp(uint8_t d){
	uint16_t result = 0;
	if(d&0x8)	result += 5000;
	if(d&0x4)	result += 2500;
	if(d&0x2)	result += 1250;
	if(d&0x1)	result += 625;
	return result;
}// dp(uint8_t d)

// this function is called by printf as a stream handler
int lcd_printf(char var, FILE *stream) {
    lcd_putc((uint8_t)var);
    return 0;
}

// this function is called by printf as a stream handler
int ser_printf(char var, FILE *stream) {
    SerialWrite((uint8_t)var);
    return 0;
}
