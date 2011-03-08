#include "adxl345.h"

#define ADXL345_W	0b00111010
#define ADXL345_R	0b00111011

int16_t read_adxl345(char reg_adr)
{		
	char lsb, msb;

	i2cSendStart();
	i2cWaitForComplete();
	
	i2cSendByte(ADXL345_W);	// write to this I2C address, R/*W cleared
	i2cWaitForComplete();
	
	i2cSendByte(reg_adr);	//Read from a given address
	i2cWaitForComplete();
	
	i2cSendStart();
	
	i2cSendByte(ADXL345_R); // read from this I2C address, R/*W Set
	i2cWaitForComplete();
	
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte(); //Read the LSB data
	i2cWaitForComplete();

	i2cReceiveByte(FALSE);
	i2cWaitForComplete();
	msb = i2cGetReceivedByte(); //Read the MSB data
	i2cWaitForComplete();
	
	i2cSendStop();
	
	return( (msb<<8) | lsb);
}

//Setup ADXL345 for constant measurement mode
void init_adxl345(void)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte(ADXL345_W); //write to ADXL345
	i2cWaitForComplete();

	i2cSendByte(0x2D); //Write to Power CTL register
	i2cWaitForComplete();

	i2cSendByte( (1<<3) ); //Set the measure bit on D3
	i2cWaitForComplete();

	i2cSendStop();
}