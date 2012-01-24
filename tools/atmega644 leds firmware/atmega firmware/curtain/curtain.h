/*
 * Copyright (c) 2009 by David Gräff
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

#ifndef MOTORCURTAIN_H
#define MOTORCURTAIN_H
#include <inttypes.h>


#define CURTAIN_COMPLETE_SENSOR_CHANGES 14
#define CURTAIN_SENSOR_PIN 3
#define CURTAIN_SENSOR_LIMIT_OPEN_PIN 2
#define CURTAIN_SENSOR_PORT PORTD
#define CURTAIN_SENSOR_DDR_PORT DDRD
#define CURTAIN_SENSOR_PIN_PORT PIND

#define CURTAIN_BUTTON_READPORT PINA
#define CURTAIN_BUTTON_PORT PORTA
#define CURTAIN_BUTTON_DIRECTION_PIN 2
#define CURTAIN_BUTTON_FORCEAUTO_PIN 3

#define CURTAIN_MOTOR_DIRECTION_PIN 0
#define CURTAIN_MOTOR_DIRECTION_PORT PORTA
#define CURTAIN_MOTOR_DIRECTION_DDR_PORT DDRA
// D5 -> PWM Pin
#define CURTAIN_MOTOR_ENABLE_PIN 3
#define CURTAIN_MOTOR_ENABLE_PORT PORTB
#define CURTAIN_MOTOR_ENABLE_DDR_PORT DDRB

void curtain_setPosition(uint8_t pos);
void curtain_stop(void);
uint8_t curtain_getPosition(void);
uint8_t curtain_getMax(void);
void curtain_init(void);
void curtain_process(void);

enum direnum {
    DirectionUp,
    DirectionDown
};

void setMotor(uint8_t speed, enum direnum direction);

#endif /* MOTORCURTAIN_H */
