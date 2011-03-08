/*
    5-4-10
    Copyright Spark Fun Electronics© 2010
    Nathan Seidle
	
	Firmware for main balloon flight board. Contains all the sensors
	
	TMP102 for I2C temperature (1 sensor inside box, 1 sensor outside box)
	BMP075 for I2C pressure
	HMC5843 for I2C magneto
	ADXL345 for I2C accelerometer
	HIH-4030 for analog humidity
	TMPT6000 for analog light level
	LS20031 for GPS
	OpenLog for logging
	
	1Hz temperature X 2 (~30ms per)
	2Hz pressure?
	10Hz magneto
	10Hz accel

	2Hz humidity?
	1Hz light level

	1Hz GPS
	
	I think after a power on, some interrupt from the bootloader is causing UART problems. After
	a reset, everything is fine. Fixed in system.c
	
	Need to monitor and log batt lvl.
	
	Get reading from http://www.eol.ucar.edu/cgi-bin/weather.cgi?site=fl
	You want millibars or hectopascals.
	
*/

//Testing
#define TEMPERATURE_ON
#define PRESSURE_ON
#define MAGNETO_ON
#define HUMIDITY_ON
#define ACCEL_ON
#define GPS_ON
#define BATTLVL_ON
#define LIGHT_ON
#define LAUNCH_ON

#include "balloon-project-v10.h"

ISR(TIMER1_OVF_vect)
{
	TCNT1 = 64535; //63536 = Overflows every 1,000us. Assume the loading of TCNT1 takes a cycle or two so we do 63538
	millis_passed++;
}


int main (void)
{
    ioinit(); //Boot up defaults

	uint16_t ch;
	
	load_settings(); //Loads all the EEPROM stored settings
	
	printf("Waiting for characters...\n");

	while(1)
	{
		printf("\nz) SOTW");
		printf(" s) Settings");
		printf(" m) Time");

		#ifdef TEMPERATURE_ON
			printf("\nt) Temperature");
		#endif

		#ifdef PRESSURE_ON
			printf("\np) Pressure");
		#endif

		#ifdef GPS_ON
			printf("\ng) Read GPS");
		#endif
		
		#ifdef MAGNETO_ON
			printf("\nc) Read magnetometer");
		#endif

		#ifdef ACCEL_ON
			printf("\na) Read accel");
		#endif
		
		#ifdef HUMIDITY_ON
			printf("\nh) Check humidity");
		#endif

		#ifdef BATTLVL_ON
			printf("\nb) Check battery");
		#endif

		#ifdef NAVIGATE_ON
			printf("\n&) Begin navigate");
			printf(" @) Start from waypoint");
		#endif
		
		printf("\n>");
		
		ch = getch();
		
		if(ch == 'z')
		{
			print_sotw(); //Print the current state of disarray
		}
  
//System time
		else if(ch == 'm')
		{
			while(1)
			{
				printf("Time passed: %ld\n", millis_passed);

				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;
					
				delay_ms(1000);
			}
		}

//Temperature
#ifdef TEMPERATURE_ON
		else if(ch == 't')
		{
			#define TEMP_EXT	1
			#define TEMP_INT	2
			int16_t temp_external, temp_internal;

			while(1)
			{
				temp_internal = tmp102Read(TEMP_INT);
				temp_external = tmp102Read(TEMP_EXT);
				
				printf("TI:%d ", temp_internal);
				printf("TE:%d ", temp_external);

				printf("\n");

				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;

				delay_ms(100);
			}
		}


		else if(ch == 'y') //Read the LM335A temp sensor
		{
			uint16_t temp, band_gap;
			long temp_k;

			while(1)
			{
				temp = get_adc(0);
				//Convert reading to mV
				//VCC = 4.92V
				//Band gap = 1.1V
				//GND = 0

				band_gap = get_adc(14);
				
				printf("TLM:%04d ", temp);
				printf("BG:%04d ", band_gap);
				temp_k = temp * (long)492; //619*492 = 304548 Board is running at 4.92V, change to 3.3V soon
				temp_k = temp_k / 1024; //304548 / 1024 = 297.4
				printf("TK:%04ld ", temp_k);
				temp = temp_k - 273;
				printf("TC:%04d ", temp);
				
				printf("\n");

				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;

				delay_ms(50);
			}
		}

#endif

//Battery level monitoring
#ifdef BATTLVL_ON
		else if(ch == 'b') //Read the battery level
		{
			long batt_lvl;
			uint16_t band_gap;

			while(1)
			{
				batt_lvl = get_adc(6); //Check raw batt voltage across a 10k/10k divider
				band_gap = get_adc(14); //Should always be 1.1V

				//Convert reading to mV
				//batt_lvl *= 330;
				//batt_lvl /= 512; //Normally you would divide by 1024, but we need to double the output because we have a 10k/10k voltage divider
				printf("B:%04ld ", batt_lvl);

				batt_lvl *= 110; //Band gap should be 1.1V
				batt_lvl /= band_gap;
				batt_lvl *= 2; //Correct for the voltage divider

				printf("BG:%04d ", band_gap);
				printf("Batt lvl:%03d\n", (uint16_t)batt_lvl);
				
				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;

				delay_ms(50);
			}
		}

#endif

//Light level
#ifdef LIGHT_ON
		else if(ch == 'l') //Read the light level
		{
			uint16_t light_lvl;

			while(1)
			{
				light_lvl = get_adc(7); //

				printf("Light lvl:%04d\n", light_lvl);
				
				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;

				delay_ms(50);
			}
		}

#endif

//Pressure
#ifdef PRESSURE_ON
		else if(ch == 'p')
		{
			#define BMP085_XCLR	3 //PortD 3
			
			long temperature = 0;
			long pressure = 0;
			//long altitude = 0;
			//double temp = 0;
			
			//Set XCLR high (pulling it low resets the BMP085)
			sbi(PORTD, BMP085_XCLR);

		i2cInit();
		delay_ms(100);

			bmp085Calibration();
			
			while(1)
			{
				bmp085Convert(&temperature, &pressure);
				
				printf("Time: %ld,", millis_passed);
				printf("T:%ld,", temperature);
				printf("P:%ld\n", pressure);

				// For fun, lets convert to altitude
				/*temp = (double) pressure/101325;
				temp = 1 - pow(temp, 0.19029);
				altitude = round(44330*temp);
				printf("A:%ld\n", altitude);*/
	
				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;
					
				delay_ms(500);
			}
		}
#endif

//Accelerometer
#ifdef ACCEL_ON
		else if(ch == 'a')
		{
			//0x31 is data_format, controls range D1/D0
			//0x2D power ctl - we have to set bit D3 to start measurment
			init_adxl345();
			
			int16_t x, y, z;
			while(1)
			{
				x = 0; y = 0; z = 0;
				#define AVG_AMT2	4
				for(char i = 0 ; i < AVG_AMT2 ; i++)
				{
					x += read_adxl345(0x32);
					y += read_adxl345(0x34);
					z += read_adxl345(0x36);
				}
				x /= AVG_AMT2; y /= AVG_AMT2; z /= AVG_AMT2;

				printf("x=%04d, y=%04d, z=%04d\n", x, y, z);
	
				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;
					
				//delay_ms(100);
			}
		}
#endif

//Humidity
#ifdef HUMIDITY_ON
		else if(ch == 'h')
		{
			uint16_t humidity_level;

			while(1)
			{
				humidity_level = read_humidity();
				
				printf("humidity=%d\n", humidity_level);
	
				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;
					
				delay_ms(250);
			}
		}
#endif

//Magnetometer
#ifdef MAGNETO_ON
		else if(ch == 'c')
		{
			int16_t x, y, z;
			int16_t xt, yt, zt;
			
			init_hmc5843(); //Setup HMC for constant measurement mode @ 50Hz

			//Read magnetometer
			while(1)
			{
				#define AVG_AMT	4
				x = 0; y = 0; z = 0;
				
				for(int i = 0 ; i < AVG_AMT ; i++)
				{
					read_hmc5843(&xt, &yt, &zt);
					x += xt; y += yt; z += zt;
					
					delay_ms(20); //Max reading is 50Hz
				}
				
				x /= AVG_AMT; y /= AVG_AMT; z /= AVG_AMT;
				
				printf("x=%4d, y=%4d, z=%4d\n", x, y, z);

				if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters
					if(UDR0 == 'x') break;
					
				//delay_ms(100);
			}
		}
#endif

//GPS
#ifdef GPS_ON
		//Waypoint control
		else if(ch == 'g')
		{
			uint16_t long_h, long_l, lat_h, lat_l;
			uint8_t siv, fix;
			
			parse_gps();
			convert_gps();

			long_h = get_long_high();
			long_l = get_long_low();
			lat_h = get_lat_high();
			lat_l = get_lat_low();
			siv = get_siv();
			fix = get_fix();
			
			//We must display %04d for the .dddd portion to get the zeros to work correctly
			printf("Long:%03d.%04d ", long_h, long_l);
			printf("Lat:%02d.%04d ", lat_h, lat_l);
			printf("SIV:%02d Fix:%d\n", siv, fix);
		}
#endif

//Let's fly!
		else if(ch == '&' || ch == 255)
		{
			report_state();
		}

//Setting control
		else if(ch == 's')
		{
			setting_control();
		}

		else
			printf("%d\n", ch);

		//Toggle the status LED
		if (PINB & (1<<STAT3_LED))
			cbi(PORTB, STAT3_LED);
		else
			sbi(PORTB, STAT3_LED);

	}
    return(0);
}

void report_state(void)
{

#ifdef LAUNCH_ON
	//Variables
	int16_t accel_x, accel_y, accel_z;
	int16_t mag_x, mag_y, mag_z;
	int16_t temp_external, temp_internal;
	int16_t humidity_level;
	long temperature = 0;
	long pressure = 0;
	uint16_t long_h, long_l, lat_h, lat_l, altitude, gps_time;
	uint8_t siv, fix;
	char loop_number = 0; //1 is lat, 2 is long, 3 is alt, 4 is pressure
	long batt_lvl;
	uint16_t band_gap;

	#define FAST_AVG	10
	#define TEMP_EXT	1
	#define TEMP_INT	2
	#define BMP085_XCLR	3 //PORTD 3
	#define RF_ENABLE	4 //PORTD 4

	//Init everything
	init_adxl345();
	init_hmc5843(); //Setup HMC for constant measurement mode @ 50Hz

	//Set XCLR high (pulling it low resets the BMP085)
	sbi(PORTD, BMP085_XCLR);
	bmp085Calibration();
	
	sbi(PORTD, RF_ENABLE); //Set the radio modem SHDN pin high, so the modem is running

	while(1)
	{
		//Toggle status LED
		PORTB ^= (1<<5);
		
		//Switch to logger and report readings
		cbi(PORTD, MUX_A); //Clear the mux, talk to OpenLog
		cbi(PORTD, MUX_B); //Clear the mux, talk to OpenLog

		//Take 10 readings of the accel and mag
		for(char i = 0 ; i < FAST_AVG ; i++)
		{
			accel_x = read_adxl345(0x32);
			accel_y = read_adxl345(0x34);
			accel_z = read_adxl345(0x36);
			
			read_hmc5843(&mag_x, &mag_y, &mag_z);
			
			//Report everything
			printf("\n#,%d,%d,%d,", accel_x, accel_y, accel_z);
			printf("%d,%d,%d", mag_x, mag_y, mag_z);

			delay_ms(12);
		}
		
		//Time
		printf(",%ld,", millis_passed);
		//loop_until_bit_is_set(UCSR0A, TXC0); //Wait for TX buffer to complete at this UART
		delay_ms(3); //Wait for TX buffer to complete at this UART

		//GPS
		parse_gps();
		convert_gps();

		gps_time = get_time();
		long_h = get_long_high();
		long_l = get_long_low();
		lat_h = get_lat_high();
		lat_l = get_lat_low();
		siv = get_siv();
		fix = get_fix();
		altitude = get_altitude();
		
		printf("%06d,", gps_time);

		//We must display %04d for the .dddd portion to get the zeros to work correctly
		printf("%03d.%04d,", long_h, long_l);
		printf("%02d.%04d,", lat_h, lat_l);
		printf("%d,%d,%d,", altitude, siv, fix);

		//Temperature
		temp_internal = tmp102Read(TEMP_INT);
		temp_external = tmp102Read(TEMP_EXT);
		printf("%d,%d,", temp_internal, temp_external);

		//Humidity
		humidity_level = read_humidity();
		printf("%d,", humidity_level);

		//Battery level
		batt_lvl = get_adc(6); //Check raw batt voltage across a 10k/10k divider
		band_gap = get_adc(14); //Should always be 1.1V
		batt_lvl *= 110; //Band gap should be 1.1V
		batt_lvl /= band_gap;
		batt_lvl *= 2; //Correct for the voltage divider

		printf("%d,", (uint16_t)batt_lvl);

		//Pressure
		bmp085Convert(&temperature, &pressure);
		printf("%ld,", temperature);
		printf("%ld,", pressure);

		printf("*");
		delay_ms(3); //Wait to TX to complete before switching to modem

		//Switch to radio
		cbi(PORTD, MUX_A); //Set the mux, talk to radio modem
		sbi(PORTD, MUX_B); //Set the mux, talk to radio modem
		
		//Report Lat, Long, Alt, Pressure
		loop_number++;
		if(loop_number > 3) loop_number = 0;
		if(loop_number == 0) printf("%03d.%04d\n", long_h, long_l);
		if(loop_number == 1) printf("%02d.%04d\n", lat_h, lat_l);
		if(loop_number == 2) printf("%d,%d,%d\n", altitude, siv, (uint16_t)batt_lvl);
		if(loop_number == 3) 
		{
			//A weird magneto bug is popping up. The readings lock after a few 
			init_hmc5843(); //Setup HMC for constant measurement mode @ 50Hz
			printf("%d,%ld\n", temp_internal, pressure);
		}

		//Now delay for 250ms looking for incoming characters
		for(int x = 0 ; x < 250 ; x++)
		{
			if ((UCSR0A & _BV(RXC0))) //Check for incoming RX characters from radio
				if(UDR0 == 'x') return;

			delay_ms(1);
		}
	}

#endif
}

uint16_t read_humidity(void)
{
	long humid_level = get_adc(0);

	//printf("humidity=%ld ", humid_level);
	
	//Translate to percentage
	//On 5V VCC
	//0% humid = ~0.8V
	//100% = 3.8V
	//So on 3.3V VCC, 0% = 0.528V, and 100% = 2.5V
	humid_level *= 330; //420 * 330 = 138,600
	humid_level /= 1024; //138,600 / 1024 = 135 (1.350V output from sensor)
	humid_level -= 50; //1350 - 50 = 85 (0.85 on scale of 0 to 2)
	humid_level /= 2; //85 / 2 = 42 (42% humidity)
	
	return (uint16_t) humid_level;
}

