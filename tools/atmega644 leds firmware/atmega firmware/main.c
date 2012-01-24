
#include "config.h"

#include <inttypes.h>
#include <avr/io.h>
//#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>

#include "softpwm/stella.h"
#include "curtain/curtain.h"
#include "serial/usart.h"

#define soft_reset()        \
do                          \
{                           \
wdt_enable(WDTO_15MS);  \
for(;;)                 \
{                       \
}                       \
} while(0)

// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

// Function Implementation
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}

inline void sendStella(void)
{
    char data[2+STELLA_CHANNELS];
    data[0] = 'O';
    data[1] = 'L';
    for (uint8_t i=0;i<STELLA_CHANNELS;++i)
        data[i+2] = stella_getValue(i);
    FIFO_rx_slip_p(sizeof(data), data);
}

inline void sendInit(void)
{
    char data[3];
    data[0] = 'O';
    data[1] = 'I';
    data[2] = PROTOCOL_VERSION;
    FIFO_rx_slip_p(sizeof(data), data);
}


inline void sendMotor(void)
{
    char data[4];
    data[0] = 'O';
    data[1] = 'M';
    data[2] = curtain_getPosition();
    data[3] = curtain_getMax();
    FIFO_rx_slip_p(sizeof(data), data);
}

inline void sendAck(void)
{
    char data[2];
    data[0] = 'O';
    data[1] = 'K';
    FIFO_rx_slip_p(sizeof(data), data);
}

int main(void)
{
    // Alles TriState
    DDRA = 0;
    DDRB = 0;
    DDRC = 0;
    DDRD = 0;
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;

    // save power
    MCUCR |= _BV(JTD);
    PRR |= _BV(PRTWI) | _BV(PRSPI) | _BV(PRADC);
    //ADCSRA &= ~_BV(ADEN);
    //ACSR |=_BV(ACD);

    // Power Led
    DDRA|=_BV(7);
    PIN_SET(A,7);

    sei();

    // Init: Subsysteme
    uart_init();
    stella_init();
    curtain_init();
    //sensors_init();


    // Init: Variablen
    enum stateEnum {
        StateCommands,
        StateCurtainValue,
        StateLedChannel,
        StateLedValue
    } state = StateCommands;

    uint8_t slowdown_counter = 0;
    uint8_t slowdown_counter2 = 0;
    uint8_t panic_counter = 0;
    uint8_t stella_channel = 0;
    enum stella_set_function stella_fading = STELLA_SET_IMMEDIATELY;

    // Force user to set the button into start position
    while (!bit_is_set(CURTAIN_BUTTON_READPORT,CURTAIN_BUTTON_FORCEAUTO_PIN) ||
            !bit_is_set(CURTAIN_BUTTON_READPORT,CURTAIN_BUTTON_DIRECTION_PIN)
          ) {
        if (++slowdown_counter == 0) {
            stella_setValue(STELLA_SET_IMMEDIATELY, 0, ++slowdown_counter2);
        }
        stella_process();
    }
    slowdown_counter = 0;
    slowdown_counter2 = 0;
    stella_setValue(STELLA_SET_IMMEDIATELY, 0, 0);
    stella_process();
    uint8_t button_laststate=255;
    uint8_t button_state=255;

    // Hauptprogramm
    while (1) {
        stella_process();
        curtain_process();
        //sensors_process();

        //panic counter
        if (++slowdown_counter == 0) {
            // Force position if button is set
            button_state = (CURTAIN_BUTTON_READPORT & _BV(CURTAIN_BUTTON_DIRECTION_PIN)) |
                           (CURTAIN_BUTTON_READPORT & _BV(CURTAIN_BUTTON_FORCEAUTO_PIN));
            if (button_state != button_laststate) {
                button_laststate = button_state;
                if (button_state & _BV(CURTAIN_BUTTON_FORCEAUTO_PIN))
                {
                    curtain_stop();
                } else {
                    if (button_state & _BV(CURTAIN_BUTTON_DIRECTION_PIN)) {
                        curtain_setPosition(0);
                    } else {
                        curtain_setPosition(255);
                    }
                }
            }

            if (++slowdown_counter2==0) {
                if (panic_counter == 110) {
                    ++panic_counter;
                    // activate first led to indicate a broken connection to the server
                    for (uint8_t i=0;i<STELLA_CHANNELS;++i) {
                        stella_setValue(STELLA_SET_FADE, i, 0);
                    }
                    stella_setValue(STELLA_SET_FADE, 0, 255);
                    PIN_SET(A,6);
                } if (panic_counter == 111) {
                    stella_setValue(STELLA_SET_FADE, 0, stella_getValue(0)?0:255);
                } else
                    ++panic_counter;
            }
        }

        while ((UCSR0A & (1 << RXC0))) {
            //buffersize--;
            // reset panic counter
            if (panic_counter) {
                sendAck();
                stella_setValue(STELLA_SET_IMMEDIATELY, 0, 0);
                PIN_CLEAR(A,6);
                panic_counter = 0;
            }
            const uint8_t id = UDR0;
            switch (state) {
            case StateCommands:
                switch (id) {
                case 0xef: // get values
                    sendInit();
                    sendAck();
                    sendStella();
                    sendAck();
                    //sendSensor();
                    sendMotor();
                    sendAck();
                    continue;
                case 0xdf: // prepare for a curtain value
                    state = StateCurtainValue;
                    continue;
                case 0xcf: // prepare for a led channel and value (fade:immediately)
                    state = StateLedChannel;
                    stella_fading = STELLA_SET_IMMEDIATELY;
                    continue;
                case 0xbf: // prepare for a led channel and value (fade:fade)
                    state = StateLedChannel;
                    stella_fading = STELLA_SET_FADE;
                    continue;
                case 0xaf: // prepare for a led channel and value (fade:fade flashy)
                    state = StateLedChannel;
                    stella_fading = STELLA_SET_FLASHY;
                    continue;
                case 0x9f: // prepare for a led channel and value (fade:fade immediately+relative)
                    state = StateLedChannel;
                    stella_fading = STELLA_SET_IMMEDIATELY_RELATIVE;
                    continue;
                case 0x00: // reset panic counter and acknowdlegde
                    sendAck();
                    continue;
                }
                break;
            case StateCurtainValue:
                state = StateCommands;
                curtain_setPosition(id);
                break;
            case StateLedChannel:
                state = StateLedValue;
                stella_channel = id;
                break;
            case StateLedValue:
                state = StateCommands;
                stella_setValue(stella_fading, stella_channel, id);
                break;
            }
        }
    }
    return 0;
}
