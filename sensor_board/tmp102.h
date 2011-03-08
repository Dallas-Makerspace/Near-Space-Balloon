/*
    BMP085 Test Code
	5-7-10
    Copyright Spark Fun Electronics© 2010
    Nathan Seidle
	
	Example code for TMP102 temperature sensor

	TMP102 powers up in temperature mode, all we have to do is:
	Write to 0x48 (I2C Address of TMP102 when the AD0 pin is pulled to ground)
	Write to 0x00 (Pointer to Temp register)
	Read 2 bytes from 0x48 (Get Temp high byte and low byte)

*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include "types.h"
#include "defs.h"
//#include "i2c.h"

#define FALSE	0
#define TRUE	-1

#define TMP102_EXT_ADR 0b01001000 //External sensor with AD0 grounded
#define TMP102_EXT_R ( (TMP102_EXT_ADR << 1) | 0x01)
#define TMP102_EXT_W ( (TMP102_EXT_ADR << 1) & 0xFE)

#define TMP102_INT_ADR 0b01001001 //Internal sensor with ADO to VCC
#define TMP102_INT_R ( (TMP102_INT_ADR << 1) | 0x01)
#define TMP102_INT_W ( (TMP102_INT_ADR << 1) & 0xFE)

int16_t tmp102Read(char temp_number);

// I2C Functions
void i2cSendStart(void);
void i2cSendStop(void);
void i2cWaitForComplete(void);
void i2cSendByte(unsigned char data);
void i2cReceiveByte(unsigned char ackFlag);
unsigned char i2cGetReceivedByte(void);
void delay_ms(uint16_t x);
