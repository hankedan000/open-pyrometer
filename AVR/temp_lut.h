#ifndef TEMP_LUT_H
#define TEMP_LUT_H

/*
	This file contains data values to interpolate a fahrenheit vs. ADC curve for a
100Kohm Murata #NXFT15WF104FA1B025 thermistor. These data points correspond to the
slopes and y-intercepts of 32 line segments that approximate the original curve.
*/

const uint16_t slope[] = {	0x0000,
							0x4FEA,
							0x33E5,
							0x27FC,
							0x215D,
							0x1D30,
							0x1A5B,
							0x185B,
							0x16EA,
							0x15DE,
							0x1520,
							0x14A0,
							0x1453,
							0x1435,
							0x1441,
							0x1478,
							0x14D9,
							0x1569,
							0x162C,
							0x172B,
							0x1874,
							0x1A18,
							0x1C34,
							0x1EF1,
							0x2292,
							0x2786,
							0x2E93,
							0x3940,
							0x4B03,
							0x6DAF,
							0xCC37,
							0x0000};
							
const int16_t y_int[] = {	0x0000,
							0xFE25,
							0xFF05,
							0xFF94,
							0xFFFE,
							0x0053,
							0x0096,
							0x00CE,
							0x00FD,
							0x0122,
							0x0140,
							0x0156,
							0x0164,
							0x016B,
							0x0168,
							0x015B,
							0x0143,
							0x011D,
							0x00E6,
							0x009A,
							0x0033,
							0xFFA8,
							0xFEEF,
							0xFDF3,
							0xFC96,
							0xFAA7,
							0xF7CA,
							0xF349,
							0xEB83,
							0xDBCE,
							0xAF7E,
							0x0000};
#endif
