#include "hmc5843.h"

void read_hmc5843(int16_t* x, int16_t* y, int16_t* z)
{		
	uint8_t xh, xl, yh, yl, zh, zl;
	
	//must read all six registers plus one to move the pointer back to 0x03
	i2cSendStart();
	i2cWaitForComplete();
	i2cSendByte(0x3D); //read from HMC
	i2cWaitForComplete();

	xh = hmc_getbyte();	//x high byte
	xl = hmc_getbyte();	//x low byte
	yh = hmc_getbyte();	//y high byte
	yl = hmc_getbyte();	//y low byte
	zh = hmc_getbyte();	//z high byte
	zl = hmc_getbyte();	//z low byte
	
	i2cSendByte(0x3D); //must reach 0x09 to go back to 0x03
	i2cWaitForComplete();
	
	i2cSendStop();	

	*x = (xh << 8)|xl;
	*y = (yh << 8)|yl;
	*z = (zh << 8)|zl;
}

//Just pulls one byte off the I2C bus, assume ack = TRUE
uint8_t hmc_getbyte(void)
{
	uint8_t temp;
	
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();
	temp = i2cGetReceivedByte(); //Get the byte
	i2cWaitForComplete();

	return(temp);
}

//Setup HMC for constant measurement mode
void init_hmc5843(void)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte(0x3C); //write to HMC
	i2cWaitForComplete();

	i2cSendByte(0x00); //Write to configuration register A
	i2cWaitForComplete();

	i2cSendByte(0b00011000); //Register A: 50Hz data output rate
	i2cWaitForComplete();

	i2cSendByte(0b00100000); //Register B: default
	i2cWaitForComplete();

	i2cSendByte(0x00);    //Mode register: continuous measurement mode
	i2cWaitForComplete();

	i2cSendStop();
}