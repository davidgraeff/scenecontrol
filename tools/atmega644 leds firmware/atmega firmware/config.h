// inkrement only if changes in the protocol were made
#define PROTOCOL_VERSION 2

#ifndef F_CPU
#define F_CPU 18432000UL
#endif

#define BAUD 115200
//115200 57600
//#define UART_BUFFER  80 

#define _TCCR2_PRESCALE TCCR2B
#define _OUTPUT_COMPARE_IE2 OCIE2B
#define _OUTPUT_COMPARE_REG2 OCR2B
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMPB_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK2
#define _TIFR_TIMER2 TIFR2
#define _CS20 CS20
#define _CS21 CS21
#define _CS22 CS22
#define _COM20 COM2B0
#define _COM21 COM2B1
#define _WGM20 WGM20
#define _WGM21 WGM21
#define _TCNT2 TCNT2

#define STELLA_PRESCALER   		_TCCR2_PRESCALE
#define STELLA_TIMSK       		_TIMSK_TIMER2
#define STELLA_CS0         		CS20
#define STELLA_CS2         		CS22
#define STELLA_TOIE        		TOIE2
#define STELLA_COMPARE_IE  		_OUTPUT_COMPARE_IE2
#define STELLA_COMPARE_VECTOR	_VECTOR_OUTPUT_COMPARE2
#define STELLA_OVERFLOW_VECTOR  _VECTOR_OVERFLOW2
#define STELLA_COMPARE_REG 		_OUTPUT_COMPARE_REG2

#define STELLA_PINS_PORT1 5
#define STELLA_OFFSET_PORT1 0
#define STELLA_PORT1 PORTC
#define STELLA_DDR_PORT1 DDRC
//#define STELLA_HIGHFREQ
#define STELLA_FADE_STEP_INIT 10

#define SENSOR_COUNT 0
#define SENSOR_STARTPIN 0
#define SENSORS_PORT PORTD
#define SENSORS_DDR_PORT DDRD
#define SENSORS_PIN_PORT PIND
#define SENSORS_PULLUP

#define HEX__(n) 0x##n##LU 
/* 8-bit conversion function */ 
#define B8__(x) ((x&0x0000000FLU)?1:0)     \
+((x&0x000000F0LU)?2:0)     \
+((x&0x00000F00LU)?4:0)     \
+((x&0x0000F000LU)?8:0)     \
+((x&0x000F0000LU)?16:0)    \
+((x&0x00F00000LU)?32:0)    \
+((x&0x0F000000LU)?64:0)    \
+((x&0xF0000000LU)?128:0) 
#define B8(d) ((unsigned char)B8__(HEX__(d))) 

/*#define _PORT_CHAR(character) PORT ## character
#define PORT_CHAR(character) _PORT_CHAR(character)*/
#define PIN_CLEAR(port,pin) (PORT ## port) &= ~_BV(pin)
#define PIN_SET(port,pin) (PORT ## port) |= _BV(pin)
