/*

*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "i2c.h" //Needed  for I2C sensors

#define FALSE 0
#define TRUE -1

#define STEER_DIR	PORTB4
#define DRIVE_DIR	PORTB3
#define STEER_CTL	PORTB2

#define DRIVE_PWM	OCR1A
#define STEER_PWM	OCR1B

#define FORWARD	1
#define REVERSE	2

#define MUSIC_RIGHT	1
#define MUSIC_LEFT	2
#define MUSIC_SONG	3
#define MUSIC_HONK	4

#define CROSS_LAT	1
#define CROSS_LONG	2

#define STEER_CENTER	1
#define STEER_LEFT		2
#define STEER_RIGHT		3

//Global variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
unsigned long millis_passed; //Total system time in milliseconds


uint16_t setting_example1; //Example universal variable
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Function prototypes
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "system.c" //I/O Init, delay_ms, EEPROM, etc
//#include "navigate.h" //Software control loop for reading compass and maintaining a given direction
//#include "i2c-compass.h" //This is the software I2C routines for the HMC6352 digital compass
#include "gps.h" //All GPS parsing - gives long/lat/lock/siv
//#include "proximity.h" //Checks all the ultrasonic range sensors
//#include "accel.c" //Handles the accelerometer ADC
#include "bmp085.h" //I2C Pressure conversion routines
#include "tmp102.h" //I2C Temperature
#include "hmc5843.h" //I2C Magneto
#include "adxl345.h" //I2C Accel


void report_state(void);
uint16_t read_humidity(void);
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
