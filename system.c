
#include "system.h"
#include "gps.h" //All GPS parsing - gives long/lat/lock/siv

//Global variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define SETTING_EXP	40

//#define HEAD_start		100 //Headings take 2 spots each and we have 10 headings to go with 10 way points, so 20 spots
//#define CROSS_start		150 //Cross types take 1 spot each and we have 10 cross types to go with 10 way points, so 10 spots

void EEPROM_write(uint16_t uiAddress, unsigned char ucData)
{
	while(EECR & (1<<EEPE)); //Wait for completion of previous write
	EEARH = uiAddress >> 8; //Set up address and data registers
	EEARL = uiAddress; //Set up address and data registers
	EEDR = ucData;
	EECR |= (1<<EEMPE); //Write logical one to EEMWE
	EECR |= (1<<EEPE); //Start eeprom write by setting EEWE
}

unsigned char EEPROM_read(uint16_t uiAddress)
{
	while(EECR & (1<<EEPE)); //Wait for completion of previous write
	EEARH = uiAddress >> 8; //Set up address and data registers
	EEARL = uiAddress; //Set up address and data registers
	EECR |= (1<<EERE); //Start eeprom read by writing EERE
	return EEDR; //Return data from data register
}

//Reads two sequential 8 bit values from a given I2C address
//Returns one 16 bit value
uint16_t i2cRead_16(unsigned char i2c_address, unsigned char register_address)
{
	char msb, lsb;
	uint16_t data;
	
	i2cSendStart();
	i2cWaitForComplete();
	
	i2cSendByte(i2c_address<<1 & 0x0FE);	// write to this I2C address, R/*W cleared
	i2cWaitForComplete();
	
	i2cSendByte(register_address);	// write register address
	i2cWaitForComplete();
	
	i2cSendStart();
	
	i2cSendByte(i2c_address<<1 | 0x01); // read from this I2C address, R/*W Set
	i2cWaitForComplete();
	
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte();	// Get MSB result
	i2cWaitForComplete();
	
	i2cReceiveByte(FALSE);
	i2cWaitForComplete();
	msb = i2cGetReceivedByte();	// Get LSB result
	i2cWaitForComplete();
	
	i2cSendStop();
	
	data = msb << 8;
	data |= lsb;
	
	return data;
}

void ioinit(void)
{
    millis_passed = 0;
	
	//1 = output, 0 = input 
    DDRB = 0xFF; //Assume all pins are outputs
	DDRC = 0xFF;
	DDRD = 0xFF;

	//Set any inputs
	#define HUMID		0
	#define	BATT_LVL	6
	#define LIGHT_LVL	7
    DDRC &= ~( (1<<HUMID)|(1<<BATT_LVL)|(1<<LIGHT_LVL) );

	#define BMP085_EOC	2
	DDRD &= ~(1<<BMP085_EOC);

    //Setup USART baud rate
    UBRR0H = SERIAL_MYUBRR >> 8;
    UBRR0L = SERIAL_MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0); //No receive interrupt
	UCSR0A &= ~(1<<U2X0); //This clears the double speed UART transmission that may be set by the Arduino bootloader

    stdout = &mystdout; //Required for printf init

    //Init Timer0 for delay_us
	TCCR0B = (1<<CS01); //Set Prescaler to clk/8 : 1click = 0.5us(assume we are running at external 16MHz). CS01=1 

    //Init Timer1 for millisecond counting
	TCCR1B = (1<<CS11); //Set Prescaler to clk/8 : 1click = 0.5us(assume we are running at external 16MHz). CS01=1 
	TIMSK1 = (1<<TOIE1); //Enable overflow interrupt
	//We will load the ISR to fire every 65536 - 2000 = 63,536, which should be every 1ms.
	
	//Enable ADC - 16MHz / 128 = 125khz (good)
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	//ADCSRA = (1<<ADEN)|(1<<ADPS0);
	ADMUX = (1<<REFS0); //Use AVcc as reference

	/*for(uint8_t i = 0 ; i < 6 ; i++)
	{
		sbi(PORTD, STAT1_LED);
		delay_ms(25);
		
		cbi(PORTD, STAT1_LED);
		delay_ms(25);
	}*/
	
	i2cInit(); //Get the I2C bus ready

	PORTC = 0b00110000; //pullups on the I2C bus
	
	sei(); //Enable interrupts, namely the timer1 millisecond interrupt
}

//Print the state of the world array
//This array is a mix of visible ASCII and binary values
void print_sotw(void)
{
	printf("\n\n");

//Get GPS
#ifdef GPS_ON
	if(parse_gps() != 0)
	{
		printf("LA:%04d.", get_lat_high()); //GPS
		printf("%04d ", get_lat_low());

		printf("LO:%05d.", get_long_high());
		printf("%04d ", get_long_low());

		printf("FIX:%01d ", get_fix());
		printf("SIV:%01d ", get_siv());
	}
	else
		printf("!NoGPS!");
#endif

	printf("\n");
}

//Returns 16 bit digital value from ADC on given ADC channel
uint16_t get_adc(uint8_t adc_channel)
{
	#define AVG_AMT	4

	uint8_t i;
	uint16_t result = 0;
	uint16_t average = 0;
	
	ADMUX &= 0b11110000;
	ADMUX |= (adc_channel & 0x0F); //Select ADC channel
	
	delay_ms(1);

	for(i = 0 ; i < AVG_AMT ; i++)
	{
		ADCSRA |= (1<<ADIF)|(1<<ADSC); //Clear any previous ADIF by writing a 1 into ADIF and start conversion
	
		while( (ADCSRA & (1<<ADIF)) == 0); //Wait for ADC to complete
	
		result = ADCL;
		result |= (ADCH << 8);
		
		average += result;

		//delay_ms(1);
	}
	average /= AVG_AMT;

	return(average);
}


//Print the list of current settings
void setting_control(void)
{
	char ch;
	
	printf("\n\n");

	while(1)
	{
		load_settings(); //Read all the settings
		
		//Print them
		printf("1) User setting 1: %d\n", setting_example1);
		printf("x) Exit\n");
		printf("\n");

		ch = getch();
		
		if(ch == 'x') return;
		else if(ch == '1') setting_input(setting_example1, SETTING_EXP);
	}
}

//Inputs a string and records it to a given EEPROM spot
void setting_input(uint16_t setting_name, uint16_t setting_spot)
{
	uint16_t new_value;
	char buffer[24];

	printf("\n%d:", setting_name);
	read_line(buffer, sizeof(buffer));
	new_value = atoi(buffer);
	record_setting(setting_spot, new_value);
}

//Loads all current system settings from EEPROM
void load_settings(void)
{
	setting_example1 = read_setting(SETTING_EXP);

}

//Reads a 16-bit value from EEPROM
uint16_t read_setting(char setting_number)
{
	uint16_t set_value;
	set_value =  EEPROM_read(setting_number + 0);
	set_value |= EEPROM_read(setting_number + 1) << 8;
	return(set_value);
}

//Records a 16-bit value (setting_value) to a given eeprom spot (setting_number
void record_setting(char setting_number, uint16_t setting_value)
{
	EEPROM_write(setting_number + 0, setting_value & 0xFF);
	EEPROM_write(setting_number + 1, setting_value >> 8);
}

//Reads a line until the \n enter character is found
uint8_t read_line(char* buffer, uint8_t buffer_length)
{
    memset(buffer, 0, buffer_length);

    uint8_t read_length = 0;
    while(read_length < buffer_length - 1)
    {
        uint8_t c = getch();

        if(c == 0x08 || c == 0x7f)
        {
            if(read_length < 1)
                continue;

            --read_length;
            buffer[read_length] = '\0';

            putch(0x08);
            putch(' ');
            putch(0x08);

            continue;
        }

        putch(c);

        if(c == '\n' || c == '\r')
        {
            buffer[read_length] = '\0';
			break;
        }
        else
        {
            buffer[read_length] = c;
            ++read_length;
        }
    }

    return read_length;
}

//General short delays
void delay_ms(uint16_t x)
{
	for (; x > 0 ; x--)
	{
		delay_us(250);
		delay_us(250);
		delay_us(250);
		delay_us(250);
	}
}

//General short delays
void delay_us(uint16_t x)
{
	//x *= 2; //Runs at 16MHz instead of normal 8MHz

	while(x > 256)
	{
		TIFR0 = (1<<TOV0); //Clear any interrupt flags on Timer2
		TCNT0 = 0; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click
		while( (TIFR0 & (1<<TOV0)) == 0);
		
		x -= 256;
	}

	TIFR0 = (1<<TOV0); //Clear any interrupt flags on Timer2
	TCNT0 = 256 - x; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click
	while( (TIFR0 & (1<<TOV0)) == 0);
}

int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;

    return 0;
}

char getch(void)
{
	int counter = 0;
	while(!(UCSR0A & _BV(RXC0)))
	{
		if(counter++ > 3000) return(255); //Return timed out error after 3 seconds of inactivity
		delay_ms(1);
	}

	return UDR0;
}


void putch(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

