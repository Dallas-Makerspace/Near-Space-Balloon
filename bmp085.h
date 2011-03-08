/*
    BMP085 Test Code
	April 7, 2010
	by: Jim Lindblom
	
	Test code for the BMP085 Barometric Pressure Sensor.
	We'll first read all the calibration values from the sensor.
	Then the pressure and temperature readings will be read and calculated.
	Also attempts to calculate altitude (remove comments)
	The sensor is run in ultra low power mode.
	Tested on a 3.3V 8MHz Arduino Pro
	A4 (PC4) -> SDA
	A5 (PC5) -> SCL
	No Connection to EOC or XCLR pins
*/

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include "math.h"	// To calculate altitude
//#include "i2c.h"

#define BMP_EOC	2

#define FALSE	0
#define TRUE	-1

void i2cSendStart(void);
void i2cSendStop(void);
void i2cWaitForComplete(void);
void i2cSendByte(unsigned char data);
void i2cReceiveByte(unsigned char ackFlag);
unsigned char i2cGetReceivedByte(void);
void delay_ms(uint16_t x);

uint16_t i2cRead_16(unsigned char i2c_address, unsigned char register_address);

#define BMP085_ADR 0b01110111
#define BMP085_R (BMP085_ADR << 1 | 0x01) //0xEF
#define BMP085_W (BMP085_ADR << 1 & 0xFE) //0xEE
#define OSS 0	// Oversampling Setting (note: code is not set up to use other OSS values)

//#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
//#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

///============Function Prototypes=========/////////////////
void bmp085Calibration(void);
short bmp085ReadShort(unsigned char address);
uint16_t bmp085ReadTemp(void);
long bmp085ReadPressure(void);
void bmp085Convert(long * temperature, long * pressure);

/////=========Global Variables======////////////////////
short ac1;
short ac2; 
short ac3; 
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1; 
short b2;
short mb;
short mc;
short md;
