/*
    11-11-09
    Copyright Spark Fun Electronics© 2009
    Aaron Weiss
    aaron at sparkfun.com
    
    HMC5843 3-axis magnetometer
	
	ATMega328 w/ external 16MHz resonator
	High Fuse DA
    Low Fuse FF
	
	raw output in continuous mode
*/
#include <stdlib.h>
#include <stdio.h>
//#include "i2c.h"

#define FALSE	0
#define TRUE	-1

void i2cSendStart(void);
void i2cSendStop(void);
void i2cWaitForComplete(void);
void i2cSendByte(unsigned char data);
void i2cReceiveByte(unsigned char ackFlag);
unsigned char i2cGetReceivedByte(void);
void delay_ms(uint16_t x);

void init_hmc5843(void);
void read_hmc5843(int16_t* x, int16_t* y, int16_t* z);
uint8_t hmc_getbyte(void);