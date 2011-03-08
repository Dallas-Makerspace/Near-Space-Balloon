/*

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

int16_t read_adxl345(char reg_adr);
void init_adxl345(void);
