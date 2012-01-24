/*
 * Copyright (c)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "curtain.h"
#include "config.h"
#include "softpwm/stella.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


volatile enum stateenum {
    DownState,
    UpState,
    FastUpState,
    SensorNotWorkingState,
    SensorCalibratingState,
    IdleState
} state = SensorNotWorkingState;
int8_t motor_pwm_next = 0;

int8_t currentposition;
int8_t movetoposition;
int8_t inverted_direction=1;
/* Abort if runningticks is too high */
volatile uint8_t runningticksWithoutChange;
volatile uint8_t sensorChanges;
volatile uint8_t sensorLimitSwitchOpen;
uint8_t lastspeed;
enum direnum lastdirection;

inline void resetTicksCounter(void) {
    runningticksWithoutChange = 1;
    sensorChanges = 0;
}

inline void stopMotor(void)
{
    TCCR0B = 0;
    TIMSK0 = 0;
    TCCR0A = _BV(COM0A1);
    TCNT0 = 0;
    OCR0A = 0; // force 0 output
    TCCR0B |= _BV(FOC0A);
}

inline void startMotor(void)
{
    //set on upcounting, clear on downcounting || phase+freq correct pwm using OCR1A
    // FastPWM 8Bit
    TCCR0B = _BV(WGM12); //_BV(WGM13) |
    //TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM10);
    TCCR0A = _BV(COM0A1) | _BV(WGM10) ; //| _BV(WGM11)
    TIMSK0 =  _BV(TOIE0); //| _BV(OCIE1A) // _BV(TOIE1)  | _BV(OCIE1A) |
    TCCR0B |= (_BV(CS02) | _BV(CS00)); // Prescaler = 1024
}

void setMotor(uint8_t speed, enum direnum direction)
{
    // on direction change: stop motor and wait for a few msec
    if (lastdirection!=direction && speed) {
        stopMotor();
        lastspeed = 0;
        _delay_ms(30);
    }

    lastdirection = direction;

    if (lastspeed==speed)
        return;

    lastspeed = speed;

    if (speed) {
        if ((direction==DirectionDown && inverted_direction) || (direction==DirectionUp && !inverted_direction))
            CURTAIN_MOTOR_DIRECTION_PORT |= _BV(CURTAIN_MOTOR_DIRECTION_PIN);
        else
            CURTAIN_MOTOR_DIRECTION_PORT &= (uint8_t)~_BV(CURTAIN_MOTOR_DIRECTION_PIN);

        if (direction==DirectionUp && sensorLimitSwitchOpen)
            return;

        if (sensorLimitSwitchOpen && lastdirection==DirectionUp) {
            stopMotor();
            return;
        }

        resetTicksCounter();
        OCR0A = speed;
        startMotor();
    } else
        stopMotor();
}

// Finish sensor
ISR(INT0_vect) {
    sensorLimitSwitchOpen = 1;
    if (lastdirection==DirectionUp)
        setMotor(0,DirectionDown);
}

// Rotary sensor
ISR(INT1_vect) {
    // try to avoid double recognition of sensor value
    if (runningticksWithoutChange)
        ++sensorChanges;
    runningticksWithoutChange = 0;
}

ISR(TIMER0_OVF_vect) {
    ++runningticksWithoutChange;
}


inline void checkRunningTicks(void) {
    if (runningticksWithoutChange>200) {
        state = SensorNotWorkingState;
        currentposition = -1;
        setMotor(0, DirectionDown);
        stella_setValue(STELLA_SET_IMMEDIATELY, 0, 255);
        stella_process();
        return;
    }

}

void
curtain_init (void)
{
    // Inputs
    CURTAIN_SENSOR_DDR_PORT &= (uint8_t)(~_BV(CURTAIN_SENSOR_PIN)|_BV(CURTAIN_SENSOR_LIMIT_OPEN_PIN));
    DDRA &= (uint8_t)~(_BV(CURTAIN_BUTTON_DIRECTION_PIN) | _BV(CURTAIN_BUTTON_FORCEAUTO_PIN));
    // activate pullup if necessary
    CURTAIN_SENSOR_PORT |= _BV(CURTAIN_SENSOR_PIN) | _BV(CURTAIN_SENSOR_LIMIT_OPEN_PIN);
    CURTAIN_BUTTON_PORT |= _BV(CURTAIN_BUTTON_DIRECTION_PIN) | _BV(CURTAIN_BUTTON_FORCEAUTO_PIN);

    // Outputs
    CURTAIN_MOTOR_ENABLE_DDR_PORT |= _BV(CURTAIN_MOTOR_ENABLE_PIN);
    CURTAIN_MOTOR_DIRECTION_DDR_PORT |= _BV(CURTAIN_MOTOR_DIRECTION_PIN);

    // Interrupt on sensor changes
    EICRA |= _BV(ISC01) | _BV(ISC11); // falling edge;  ISC10: any logical change; (ISC11;ISC10=rising)
    EIMSK |= _BV(INT1) | _BV(INT0);

    // state
    state = SensorCalibratingState;

    // DEBUG
//     while (1) {
//         if (!bit_is_set(CURTAIN_BUTTON_READPORT,CURTAIN_BUTTON_FORCEAUTO_PIN))
//         {
//             if (!bit_is_set(CURTAIN_BUTTON_READPORT,CURTAIN_BUTTON_DIRECTION_PIN)) {
//                 setMotor(50, DirectionOpening);
//                 stella_setValue(STELLA_SET_IMMEDIATELY, 1, 230);
//             } else {
//                 setMotor(50, DirectionClosing);
//                 stella_setValue(STELLA_SET_IMMEDIATELY, 1, 100);
//             }
//         } else {
//             setMotor(0,DirectionOpening);
//             stella_setValue(STELLA_SET_IMMEDIATELY, 1, 30);
//         }
//
//         if (sensorChanges) {
//             sensorChanges=0;
//             stella_setValue(STELLA_SET_IMMEDIATELY, 0, stella_getValue(0)?30:0);
//         } else
// 	  stella_setValue(STELLA_SET_IMMEDIATELY, 0, 0);
//         stella_process();
//     }

    // First close curtain a little bit
    if (!sensorLimitSwitchOpen) {
        sensorLimitSwitchOpen = 0;
        setMotor(40, DirectionDown);
        while (sensorChanges<3 && state==SensorCalibratingState) {
            checkRunningTicks();
        }
        sensorLimitSwitchOpen = 0;
        setMotor(40, DirectionUp);

        // check if sensor pin changes its value and open curtain for CURTAIN_COMPLETE_SENSOR_CHANGES times (assumption: does not damage curtain)
        while (!sensorLimitSwitchOpen && state==SensorCalibratingState) {
            checkRunningTicks();
        }

        // stop motor in every case at the end of the calibration
        setMotor(0, DirectionUp);
    }

    if (state == SensorCalibratingState)
        state = IdleState;
}

void
curtain_process(void)
{
    checkRunningTicks();

    if (sensorChanges)
    {
        // next action depend on the current state
        switch ( state ) {
        case DownState:

            // reset limit-open sensor value
            sensorLimitSwitchOpen = 0;
            currentposition += sensorChanges;
            if (currentposition>=movetoposition) {
                currentposition = movetoposition;
                state = IdleState;
                setMotor(0, DirectionDown);
            }
            break;
        case UpState:
            currentposition -= sensorChanges;

            if (sensorLimitSwitchOpen || currentposition<movetoposition) {
                currentposition = movetoposition;
                state = IdleState;
                setMotor(0, DirectionDown);
            }
            break;
        case FastUpState:
            currentposition -= sensorChanges;

            if (sensorLimitSwitchOpen || currentposition<movetoposition) {
                currentposition = movetoposition;
                state = IdleState;
                setMotor(0, DirectionDown);
            }
            break;
        default:
            setMotor(0, DirectionDown);
            break;
        };
        sensorChanges = 0;
    }
}

// Großer Wert = Zu
void curtain_setPosition(uint8_t pos)
{
    // Do not accept new positions if the module is currently reseting to the start position
    // or the sensor is not working or pos is an invalid value
    if (state == SensorNotWorkingState || state == SensorCalibratingState)
        return;

    // apply maximum value
    if (pos > CURTAIN_COMPLETE_SENSOR_CHANGES)
        pos = CURTAIN_COMPLETE_SENSOR_CHANGES;

    setMotor(0, DirectionDown);
    movetoposition = currentposition;

    // Down: high value
    if (pos > currentposition) {
        state = DownState;
        movetoposition = pos;
        setMotor(255, DirectionDown);
    } else if (pos < currentposition) { // Up: low value
        movetoposition = pos;
        if (currentposition-pos>2) {
            state = FastUpState;
            setMotor(255, DirectionUp);
        }
        else {
            state = UpState;
            setMotor(60, DirectionUp);
        }
    }
}

void curtain_stop()
{
    setMotor(0, DirectionDown);
    movetoposition = currentposition;
}

inline
uint8_t curtain_getMax(void)
{
    return (uint8_t)CURTAIN_COMPLETE_SENSOR_CHANGES;
}

inline
uint8_t curtain_getPosition(void)
{
    return currentposition;
}

