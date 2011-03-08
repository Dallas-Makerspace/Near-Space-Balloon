/*
    TMP Test Code
	5-31-10
    Copyright Spark Fun Electronics© 2010
    Nathan Seidle
	
	Example code for the TMP102 11-bit I2C temperature sensor
	
	You will need to connect the ADR0 pin to one of four places. This code assumes ADR0 is tied to VCC. 
	This results in an I2C address of 0x93 for read, and 0x92 for write.
	
	This code assumes regular 12 bit readings. If you want the extended mode for higher temperatures, the code
	will have to be modified slightly.

*/

#include "tmp102.h"

#define TEMP_EXT	1
#define TEMP_INT	2

#define TEMP_REG 	0x00

//Read a tmp102 sensor on a given temp_number or channel
int16_t tmp102Read(char temp_number)
{

	uint8_t msb, lsb;
	int16_t temp;
	
	i2cSendStart();	
    i2cWaitForComplete();
	
	//i2cSendByte(TMP_WR); //We want to write a value to the TMP
	if(temp_number == TEMP_EXT) i2cSendByte(TMP102_EXT_W); //Write to TMP102 External
	if(temp_number == TEMP_INT) i2cSendByte(TMP102_INT_W); //Write to TMP102 Internal

	i2cWaitForComplete();

	i2cSendByte(TEMP_REG); //Set pointer regster to temperature register (it's already there by default, but you never know)
	i2cWaitForComplete();
	
	i2cSendStart();
	
	//i2cSendByte(TMP_RD); // Read from this I2C address, R/*W Set
	if(temp_number == TEMP_EXT) i2cSendByte(TMP102_EXT_R);	//Read from TMP102 External
	if(temp_number == TEMP_INT) i2cSendByte(TMP102_INT_R);	//Read from TMP102 Internal
	i2cWaitForComplete();
	
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();
	msb = i2cGetReceivedByte(); //Read the MSB data
	i2cWaitForComplete();

	i2cReceiveByte(FALSE);
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte(); //Read the LSB data
	i2cWaitForComplete();
	
	i2cSendStop();
	
	//printf("0x%02X ", msb);
	//printf("0x%02X ", lsb);
	
	//Test
	//msb = 0b11100111;
	//lsb = 0b00000000; //From the datasheet, -25C
	
	temp = (msb<<8) | lsb;
	temp >>= 4; //The TMP102 temperature registers are left justified, correctly right justify them
	
	//The tmp102 does twos compliment but has the negative bit in the wrong spot, so test for it and correct if needed
	if(temp & (1<<11))
		temp |= 0xF800; //Set bits 11 to 15 to 1s to get this reading into real twos compliment

	//printf("%02d\n", temp);

	//But if we want, we can convert this directly to a celsius temp reading
	//temp *= 0.0625; //This is the same as a divide by 16
	//temp >>= 4; //Which is really just a shift of 4 so it's much faster and doesn't require floating point
	//Shifts may not work with signed ints. Let's do a divide instead
	temp /= 16; //Which is really just a shift of 4 so it's much faster and doesn't require floating point

	return(temp);
}