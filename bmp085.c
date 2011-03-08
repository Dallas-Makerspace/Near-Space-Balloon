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
	
	Get reading from http://www.eol.ucar.edu/cgi-bin/weather.cgi?site=fl
	You want millibars or hectopascals.
*/

#include "bmp085.h"

void bmp085Calibration(void)
{
	/*printf("\nCalibration Information:\n");
	printf("------------------------\n");*/
	ac1 = bmp085ReadShort(0xAA);
	ac2 = bmp085ReadShort(0xAC);
	ac3 = bmp085ReadShort(0xAE);
	ac4 = bmp085ReadShort(0xB0);
	ac5 = bmp085ReadShort(0xB2);
	ac6 = bmp085ReadShort(0xB4);
	b1 = bmp085ReadShort(0xB6);
	b2 = bmp085ReadShort(0xB8);
	mb = bmp085ReadShort(0xBA);
	mc = bmp085ReadShort(0xBC);
	md = bmp085ReadShort(0xBE);
	
	/*printf("\tAC1 = %d\n", ac1);
	printf("\tAC2 = %d\n", ac2);
	printf("\tAC3 = %d\n", ac3);
	printf("\tAC4 = %d\n", ac4);
	printf("\tAC5 = %d\n", ac5);
	printf("\tAC6 = %d\n", ac6);
	printf("\tB1 = %d\n", b1);
	printf("\tB2 = %d\n", b2);
	printf("\tMB = %d\n", mb);
	printf("\tMC = %d\n", mc);
	printf("\tMD = %d\n", md);
	printf("------------------------\n\n");*/
}

// bmp085ReadShort will read two sequential 8-bit registers, and return a 16-bit value
// the MSB register is read first
// Input: First register to read
// Output: 16-bit value of (first register value << 8) | (sequential register value)
short bmp085ReadShort(unsigned char address)
{
	char msb, lsb;
	short data;
	
	i2cSendStart();
	i2cWaitForComplete();
	
	i2cSendByte(BMP085_W);	// write 0xEE
	i2cWaitForComplete();
	
	i2cSendByte(address);	// write register address
	i2cWaitForComplete();
	
	i2cSendStart();
	
	i2cSendByte(BMP085_R);	// write 0xEF
	i2cWaitForComplete();
	
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();
	msb = i2cGetReceivedByte();	// Get MSB result
	i2cWaitForComplete();
	
	i2cReceiveByte(FALSE);
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte();	// Get LSB result
	i2cWaitForComplete();
	
	i2cSendStop();
	
	data = msb << 8;
	data |= lsb;
	
	return data;
}

uint16_t bmp085ReadTemp(void)
{
	i2cSendStart();
	i2cWaitForComplete();
	
	i2cSendByte(BMP085_W);	// write 0xEE
	i2cWaitForComplete();
	
	i2cSendByte(0xF4);	// write register address
	i2cWaitForComplete();
	
	i2cSendByte(0x2E);	// write register data for temp
	i2cWaitForComplete();
	
	i2cSendStop();
	
	while ( (PIND & (1<<BMP_EOC)) == 0);// printf("!");
	//delay_ms(10);	// max time is 4.5ms
	
	return bmp085ReadShort(0xF6);
}

long bmp085ReadPressure(void)
{
	long pressure = 0;
	
	i2cSendStart();
	i2cWaitForComplete();
	
	i2cSendByte(BMP085_W);	// write 0xEE
	i2cWaitForComplete();
	
	i2cSendByte(0xF4);	// write register address
	i2cWaitForComplete();
	
	i2cSendByte(0x34);	// write register data for temp
	i2cWaitForComplete();
	
	i2cSendStop();
	
	while ( (PIND & (1<<BMP_EOC)) == 0);
	//delay_ms(10);	// max time is 4.5ms
	
	pressure = bmp085ReadShort(0xF6);
	pressure &= 0x0000FFFF;
	
	return pressure;
	
	//return (long) bmp085ReadShort(0xF6);
}

void bmp085Convert(long* temperature, long* pressure)
{
	uint16_t ut;
	long up;
	long x1, x2, b5, b6, x3, b3, p;
	unsigned long b4, b7;
	
	ut = bmp085ReadTemp();
	up = bmp085ReadPressure();
	
	//printf("ut:%d ", ut);
	//printf("up:%ld ", up);
	
	x1 = ((long)ut - ac6) * ac5 >> 15;
	x2 = ((long) mc << 11) / (x1 + md);
	b5 = x1 + x2;
	*temperature = (b5 + 8) >> 4;
	
	b6 = b5 - 4000;
	x1 = (b2 * (b6 * b6 >> 12)) >> 11;
	x2 = ac2 * b6 >> 11;
	x3 = x1 + x2;
	b3 = (((int32_t) ac1 * 4 + x3) + 2)/4;
	x1 = ac3 * b6 >> 13;
	x2 = (b1 * (b6 * b6 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;
	b7 = ((unsigned long) up - b3) * (50000 >> OSS);
	p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	*pressure = p + ((x1 + x2 + 3791) >> 4);
}
