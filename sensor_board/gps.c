
/*
	GPS file parses the 19200bps stream from a 5Hz gps Locosys module at 19200bps. Awesome stuff.
	
	Unit seems to have forgotten its settings 3-10-10
	Turning off all but GPGGA, we can run at 5Hz at 19200
	Blue line off GPS is GPS output, green is RX.
	
	Saved my life: http://www.hhhh.org/wiml/proj/nmeaxor.html
	
	
*/
#define sbi(port, pin)   ((port) |= (uint8_t)(1 << pin))
#define cbi(port, pin)   ((port) &= (uint8_t)~(1 << pin))

#include <stdio.h>
#include <avr/io.h>

#include "gps.h"

#define FOSC 8000000 //8MHz internal osc

//#define GPS_SERIAL_BAUD 57600
//#define GPS_SERIAL_MYUBRR (((((FOSC * 10) / (16L * GPS_SERIAL_BAUD)) + 5) / 10) - 1)

//#define SERIAL_BAUD 9600
//#define SERIAL_MYUBRR (((((FOSC * 10) / (16L * SERIAL_BAUD)) + 5) / 10) - 1)


//Global variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
volatile char gps_in[GPS_BUFFER_SIZE];

volatile uint16_t gps_spot = 0;
volatile uint32_t gps_time;
volatile uint16_t gps_long_high; //Degree and minute portion
volatile uint16_t gps_long_low; //Decimal minute portion
volatile uint16_t gps_lat_high;
volatile uint16_t gps_lat_low;
volatile uint8_t gps_siv;
volatile uint8_t gps_fix;
volatile uint16_t gps_altitude;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Switches MUX to listen to GPS unit for a second
//Reconfigures UART to operate at 57600 for a second
char query_gps(void)
{
	loop_until_bit_is_set(UCSR0A, UDRE0); //Wait for TX buffer to complete at this UART

	for(int i = 0 ; i < GPS_BUFFER_SIZE ; i++) gps_in[i] = 0;
	gps_spot = 0;

	sbi(PORTD, MUX_A); //Set the mux, listen to the GPS
	cbi(PORTD, MUX_B); //Set the mux, listen to the GPS
	
    /*
	The modules runs at 9600 now so we don't need this.
	UBRR0H = GPS_SERIAL_MYUBRR >> 8;
    UBRR0L = GPS_SERIAL_MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0); //No receive interrupt
	*/

//printf("1");
	
	//Start listening for $
	//If we don't hear anything for 500 cycles, forget it and bail
	uint16_t counter = 0;
	for(counter = 0 ; counter < 500 ; counter++)
	{
		if(UDR0 == '$') break;
		delay_ms(1);
	}
	if(counter == 500) 
	{
		cbi(PORTD, MUX_A); //Clear the mux, listen to the FTDI
		cbi(PORTD, MUX_B); //Clear the mux, listen to the FTDI
		//UBRR0H = SERIAL_MYUBRR >> 8;
		//UBRR0L = SERIAL_MYUBRR;
		
		printf("1!");
		
		//While the MUX_SELECT pin is set, we are receving trash on the UART from the GPS
		uint8_t temp;
		while(UCSR0A & _BV(RXC0) ) temp = UDR0; //Clear our all RX trash

		return(0);
	}

//printf("2");
	
	uint8_t break_in_two = 10;
	while(break_in_two > 0)
	{
		while(! (UCSR0A & _BV(RXC0) )) ; //Wait for incoming characters
	
		gps_in[gps_spot] = UDR0;
		gps_spot++;
		if (gps_spot > GPS_BUFFER_SIZE) gps_spot = 0;
		
		if(UDR0 == '*') break_in_two = 3;
		if(break_in_two < 4) break_in_two--;
	}

	cbi(PORTD, MUX_A); //Clear the mux, listen to the FTDI
	cbi(PORTD, MUX_B); //Clear the mux, listen to the FTDI
	//UBRR0H = SERIAL_MYUBRR >> 8;
	//UBRR0L = SERIAL_MYUBRR;
	
	//While the MUX_SELECT pin is set, we are receving trash on the UART from the GPS
	uint8_t temp;
	while(UCSR0A & _BV(RXC0) ) temp = UDR0; //Clear our all RX trash
	
	return(1);
}

//Disables the UART interrupts and parses whatever is sitting in the GPS_in buffer
uint8_t parse_gps(void)
{
	uint16_t i = 0, j = 0;
	char parse = 0;

	uint8_t sentence_checksum;

	gps_time = 0;
	gps_lat_high = 0;
	gps_lat_low = 0;
	gps_long_high = 0;
	gps_long_low = 0;
	gps_fix = 0;
	gps_siv = 99;
	gps_altitude = 0;

	//If the GPS is not responding, then bail
	if(query_gps() == 0) return(0);
	
	//print_gps_buffer();

	while(parse == 0)
	{
		//Check the check sum - don't include $ or * !
		//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
		sentence_checksum = 0;
		while(gps_in[j] != '*')
		{
			sentence_checksum ^= gps_in[j];
			j++;
		}
		
		j++; //Skip the '*'
		
		//Find what the checksum should be
		uint8_t checksum = (gps_in[j] - '0') << 4;
		j++;
		if(gps_in[j] <= '9')
			checksum |= (gps_in[j] - '0');
		else
			checksum |= (gps_in[j] - 'A' + 10);
		

		if(sentence_checksum != checksum)
		{
			//printf("Checksum failed!\n");
			parse = 4; //Bail out!
			break; //Bail out!
		}
		//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

		while(gps_in[++i] != ','); //Spin to first comma ,052644.000 (time)
		gps_time = 0;
		for(j = 0 ; j < 6 ; j++)
		{
			gps_time *= 10;
			gps_time += (gps_in[++i] - '0'); //Freaky way of converting ASCII to decimal
		}

		while(gps_in[++i] != ','); //Spin to second comma ,4000.9299 (lat)

		//Pull out lat
		gps_lat_high = 0;
		gps_lat_low = 0;
		for(j = 0 ; j < 4 ; j++)
		{
			gps_lat_high *= 10;
			gps_lat_high += (gps_in[++i] - '0');
		}

		i++; //Skip the '.'

		for(j = 0 ; j < 4 ; j++)
		{
			gps_lat_low *= 10;
			gps_lat_low += (gps_in[++i] - '0');
		}

		while(gps_in[++i] != ','); //Spin to third comma ,N (north lat)

		while(gps_in[++i] != ','); //Spin to forth comma ,10517.1101 (long)

		//Pull out long
		gps_long_high = 0;
		gps_long_low = 0;
		for(j = 0 ; j < 5 ; j++)
		{
			gps_long_high *= 10;
			gps_long_high += (gps_in[++i] - '0');
		}

		i++; //Skip the '.'

		for(j = 0 ; j < 4 ; j++)
		{
			gps_long_low *= 10;
			gps_long_low += (gps_in[++i] - '0');
		}

		while(gps_in[++i] != ','); //Spin to fifth comma ,E (east long)

		while(gps_in[++i] != ','); //Spin to sixth comma ,2 (position fix)
			
		//Pull position fix indicator
		gps_fix = gps_in[++i] - '0';

		while(gps_in[++i] != ','); //Spin to seventh comma ,7 (satellites used)

		//Pull satellies used
		gps_siv = gps_in[++i] - '0';
		if (gps_siv == 1 && gps_in[i] != ',') gps_siv = 10 + (gps_in[++i] - '0'); //Correct to 

		while(gps_in[++i] != ','); //Spin to 8th comma ,1.5 (HDOP)
		while(gps_in[++i] != ','); //Spin to 9th comma ,280.2 (Altitude)

		//Pull Altitude
		gps_altitude = 0;
		for(j = 0 ; j < 5 ; j++)
		{
			gps_altitude *= 10;
			gps_altitude += (gps_in[++i] - '0'); //Freaky way of converting ASCCII to decimal
			if(gps_in[i+1] == '.')	break;
		}

		parse = 1; //Parse complete!
	}
	
	//Reset current buffer?
	for(i = 0 ; i < GPS_BUFFER_SIZE ; i++) gps_in[i] = 0;
	gps_spot = 0;
	
	//Reset any current UART interrupts?
	i = UDR0;

	return(parse);
}

//Puts the LS20031 into 5Hz? mode, with 9600bps, and only GPGGA sentences
void configure_gps(void)
{
//Done in hyperterminal for now, but these work

//http://diydrones.com/profiles/blog/show?id=705844%3ABlogPost%3A51790&commentId=705844%3AComment%3A70322&xg_source=activity
//http://www.hhhh.org/wiml/proj/nmeaxor.html

//Turn on only GPGGA
//$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29 

//Set to 9600bps
//$PMTK251,9600*17

}

//Takes the mixed MMDD.DDDD location and makes it into MM.MMMM
void convert_gps(void)
{
	//10517.1131 = 105.28521
	//4000.9317 = 40.015528
	//Convert: http://www.csgnetwork.com/gpscoordconv.html
	//Remember, we have to display the leading zeros! Use %04d for printing
	//40.0155 is very different from 40.155
	long new_long_high, new_long_low;

	new_long_high = gps_long_high / 100; //105
	new_long_low = gps_long_high % 100; //17
	new_long_low *= 10000; //170000
	new_long_low += gps_long_low; //171131
	new_long_low /= 60; //Convert minutes to degrees
	
	gps_long_high = new_long_high; //105
	gps_long_low = new_long_low; //2852

	long new_lat_high, new_lat_low;

	new_lat_high = gps_lat_high / 100;
	new_lat_low = gps_lat_high % 100;
	new_lat_low *= 10000;
	new_lat_low += gps_lat_low;
	new_lat_low /= 60; //Convert minutes to degrees
	
	gps_lat_high = new_lat_high;
	gps_lat_low = new_lat_low;
}

//Raises the lower number by the upper number, as in 10 ^ 3 = 1000
uint16_t raise(uint16_t lower_number, uint8_t upper_number)
{
	uint16_t multiplier = lower_number;
	
	if (upper_number == 0) return(1); //Special case
	
	for(uint8_t x = 1 ; x < upper_number ; x++)
		lower_number *= multiplier;

	return(lower_number);
}

void print_gps_buffer(void)
{
	printf("GPS Array:\n");
	for(uint16_t i = 0 ; i < GPS_BUFFER_SIZE ; i++)
		printf("%c", gps_in[i]);
	printf("\n");
}


uint8_t get_siv(void)
{
	return(gps_siv);
}
uint8_t get_fix(void)
{
	return(gps_fix);
}
uint16_t get_altitude(void)
{
	return(gps_altitude);
}

uint16_t get_long_high(void)
{
	return(gps_long_high);
}

uint16_t get_long_low(void)
{
	return(gps_long_low);
}
	
uint16_t get_lat_high(void)
{
	return(gps_lat_high);
}
uint16_t get_lat_low(void)
{
	return(gps_lat_low);
}

uint32_t get_time(void)
{
	return(gps_time);
}
