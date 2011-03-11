
#define GPS_BUFFER_SIZE	150

#define MUX_A	6 //PORTD 6
#define MUX_B	7 //PORTD 7

uint8_t parse_gps(void);
char query_gps(void);
void print_gps_buffer(void);
uint16_t raise(uint16_t lower_number, uint8_t upper_number);
void convert_gps(void);

uint8_t get_siv(void);
uint8_t get_fix(void);
uint16_t get_altitude(void);
uint16_t get_long_high(void);
uint16_t get_long_low(void);
uint16_t get_lat_high(void);
uint16_t get_lat_low(void);
uint16_t get_gps_time(void);

void delay_ms(uint16_t x);
