
#include <stdlib.h>
#include "gps.h" //All GPS parsing - gives long/lat/lock/siv


#ifndef sbi
#define sbi(port, pin)   ((port) |= (uint8_t)(1 << pin))
#endif

#ifndef cbi
#define cbi(port, pin)   ((port) &= (uint8_t)~(1 << pin))
#endif

//#define FOSC 16000000 //16MHz external osc
#define FOSC 8000000 //8MHz internal osc
//#define FOSC 1000000 //1MHz internal osc

//#define SERIAL_BAUD 19200
#define SERIAL_BAUD 9600
#define SERIAL_MYUBRR (((((FOSC * 10) / (16L * SERIAL_BAUD)) + 5) / 10) - 1)

#define STAT1_LED		PORTD5 //Aka MODE_LED
//#define STAT2_LED		PB4 //Aka LOCK_LED
#define STAT3_LED		PORTB5
//#define MUX_SELECT		PD4 //Silkscreen reads 'LOCK'

//Function prototypes
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void ioinit (void);
void delay_ms(uint16_t x); // general purpose delay
void delay_us(uint16_t x);

void EEPROM_write(uint16_t uiAddress, unsigned char ucData);
unsigned char EEPROM_read(uint16_t uiAddress);

void setting_control(void);
uint16_t read_setting(char setting_number);
void record_setting(char setting_number, uint16_t setting_value);
void load_settings(void);
void setting_input(uint16_t setting_name, uint16_t setting_spot);

//static int uart_putchar(char c, FILE *stream);
int uart_putchar(char c, FILE *stream);
char getch(void);
void putch(char c);
uint8_t read_line(char* buffer, uint8_t buffer_length);

void print_sotw(void);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
