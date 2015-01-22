#ifndef MAIN_H
#define MAIN_H

#include <avr/pgmspace.h>

#define	false	0
#define False	0
#define FALSE	0
#define true	1
#define True	1
#define TRUE	1
#define UP		-1
#define DOWN	1

#define SEL_PIN		PA1
#define UP_PIN		PA3
#define DOWN_PIN	PA2
#define READ_SEL	!!(PINA&=(1<<SEL_PIN))
#define READ_UP		!!(PINA&=(1<<UP_PIN))
#define READ_DOWN	!!(PINA&=(1<<DOWN_PIN))

#define MENU_CNT		10
#define MAIN_MENU		0
#define RECORD_MENU		4
#define SAVE_MENU		8
#define SETTINGS_MENU	12
#define BATTERY_MENU	16
#define SINGLE_MENU		20
#define MEMORY_MENU		24
#define MEM_STAT_MODE	28
#define RECALL_MODE		32
#define INSTANT_MODE	36

#define SAVE_WHERE		51
#define LOAD_WHERE		52
#define MEM_CLR_MODE	53
#define TX_DATA_MODE	54

#define DRI				0
#define DRC				1
#define DRO				2
#define DFI				3
#define DFC				4
#define DFO				5
#define PFI				6
#define PFC				7
#define PFO				8
#define PRI				9
#define PRC				10
#define PRO				11

const char menu[MENU_CNT*4][9] PROGMEM = {
	"RECORD",	"RECALL",	"SETTINGS",	"",				// MAIN MENU
	"FULL SET",	"INSTANT",	"BACK",		"",				// READ MENU
	"SAVE?",	"",			"YES",		"NO",			// SAVE MENU
	"BATTERY",	"MEMORY",	"BACK",		"",				// SETTINGS MENU
	"",			"BACK",		"",			"",				// BATTERY MENU
	"",			"",			"RECORD",	"CANCEL",		// SINGLE MENU
	"USAGE",	"ERASE",	"SEND CSV",	"BACK",			// MEMORY MENU
	"",			"BACK",		"",			"",				// MEMORY STATUS MENU
	"",			"",			"",			"",				// RECALL MODE
	"",			"BACK",		"",			""				// INSTANT MODE
};
const uint8_t menu_jump_table[MENU_CNT*4] PROGMEM = {
	RECORD_MENU,	LOAD_WHERE,		SETTINGS_MENU,	MAIN_MENU,		// MAIN MENU
	SINGLE_MENU,	INSTANT_MODE,	MAIN_MENU,		MAIN_MENU,		// READ MENU
	MAIN_MENU,		MAIN_MENU,		SAVE_WHERE,		RECORD_MENU,	// SAVE MENU
	BATTERY_MENU,	MEMORY_MENU,	MAIN_MENU,		MAIN_MENU,		// SETTINGS MENU
	MAIN_MENU,		SETTINGS_MENU,	MAIN_MENU,		MAIN_MENU,		// BATTERY MENU
	MAIN_MENU,		MAIN_MENU,		SINGLE_MENU,	RECORD_MENU,	// SINGLE MENU
	MEM_STAT_MODE,	MEM_CLR_MODE,	TX_DATA_MODE,	SETTINGS_MENU,	// MEMORY MENU
	MAIN_MENU,		MEMORY_MENU,	MAIN_MENU,		MAIN_MENU,		// MEMORY STATUS MENU
	MAIN_MENU,		MAIN_MENU,		MAIN_MENU,		MAIN_MENU,		// RECALL MODE
	MAIN_MENU,		RECORD_MENU,	MAIN_MENU,		MAIN_MENU		// INSTANT MODE
};
const uint8_t menu_offset[4] = {0x01,0x41,0x0B,0x4B};
const char probe_instruction[12][10] PROGMEM = {
	"DR INNER ",
	"DR CENTER",
	"DR OUTER ",
	"DF INNER ",
	"DF CENTER",
	"DF OUTER ",
	"PF INNER ",
	"PF CENTER",
	"PF OUTER ",
	"PR INNER ",
	"PR CENTER",
	"PR OUTER "
};
const char recall_label[2][21] PROGMEM = {
	"D      FRONT       P",
	"D       REAR       P"
};
const char csv_header[] PROGMEM = "\nvvvvvvv COPY ALL TEXT BELOW HERE vvvvvvv\n,DRIVER,,,,,,PASSENGER,,,,,\n,REAR,,,FRONT,,,FRONT,,,REAR,,\nSAMPLE,INNER,CENTER,OUTER,INNER,CENTER,OUTER,INNER,CENTER,OUTER,INNER,CENTER,OUTER";
const char csv_footer[] PROGMEM = "\n^^^^^^^ COPY ALL TEXT ABOVE HERE ^^^^^^^";
const char loading_bar[] PROGMEM = "|            |";
struct menu_ptr{
	uint8_t loc;
	uint8_t min;
	uint8_t max;
	uint8_t visable;
	uint8_t orientation;
	uint8_t next_menu;
};

/*-------------------ADC to FAHRENHEIT----------------------*/
/*	Desc: Converts an ADC value to fahrenheit				*/
/*	Input: 10bit ADC measurement 'adc'						*/
/*	Returns: the corresponding fixed point fahrenheit value	*/
/*----------------------------------------------------------*/
uint32_t adc2f(uint16_t adc);

/*---------------------MODE HANDLER-------------------------*/
/*	Desc: performs additional tasks when in a menu			*/
/*----------------------------------------------------------*/
void mode_handler(void);

/*----------------------CHANGE MENU-------------------------*/
/*	Desc: menu change handler								*/
/*----------------------------------------------------------*/
void change_menu(void);

/*---------------------UPDATE POINTER-----------------------*/
/*	Desc: Updates all attributes of a menu pointer			*/
/*	Input:	a reference to a menu pointer 'ptr'				*/
/*			a byte 'dir' that tells which direction to move	*/
/*----------------------------------------------------------*/
void move_ptr(struct menu_ptr *ptr, int8_t dir);

/*--------------------DISPLAY POINTER-----------------------*/
/*	Desc: Displays a '>' next to the selected menu option	*/
/*	Input: a reference to a menu pointer 'ptr'				*/
/*----------------------------------------------------------*/
void display_ptr(struct menu_ptr *ptr);

/*----------------------DISPLAY MENU------------------------*/
/*	Desc: Displays menus to the LCD							*/
/*	Input: an 8bit menu index 'm'							*/
/*----------------------------------------------------------*/
void display_menu(uint8_t m);

/*--------------------GET BATTERY LEVEL---------------------*/
/*	Returns: a 16bit number with the battery voltage		*/
/*----------------------------------------------------------*/
uint16_t get_batt_lvl(void);

/*----------------------DECIMAL POINT-----------------------*/
/*	Desc: Converts a binary fixed point number to decimal	*/
/*	Input: a 4bit fixed point number 'd'					*/
/*	Returns:	5000 when d=0x8								*/								
/*				2500 when d=0x4								*/
/*				1875 when d=0x3								*/
/*				etc...										*/
/*----------------------------------------------------------*/
uint16_t dp(uint8_t d);

/*--------------------LCD PRINT FORMATER--------------------*/
/*	Desc: used by printf() to display stuff to LCD			*/							
/*----------------------------------------------------------*/
int lcd_printf(char var, FILE *stream);

/*------------------SERIAL PRINT FORMATER-------------------*/
/*	Desc: used by printf() to display stuff to SerialDebug	*/							
/*----------------------------------------------------------*/
int ser_printf(char var, FILE *stream);
#endif
