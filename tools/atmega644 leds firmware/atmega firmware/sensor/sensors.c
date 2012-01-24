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

#include "sensors.h"
#include "config.h"
#include <avr/io.h>

uint8_t sensor_mask;
int8_t sensor_cache;


inline void sendSensor(void)
{
    char data[3];
    data[0] = 'O';
    data[1] = 'S';
    data[2] = getSensor();
    //FIFO_rx_slip_p(sizeof(data), data);
}

inline void sensors_process(void)
{
	const int8_t sensor_value = SENSORS_PIN_PORT & sensor_mask;
	const int8_t changed = sensor_value ^ sensor_cache;
	sensor_cache = sensor_value;
	if (changed)
	  sendSensor();
}

uint8_t getSensor(void) {
  return sensor_cache;
}

inline void sensors_init (void)
{
	sensor_mask = ((1 << SENSOR_COUNT) - 1) << SENSOR_STARTPIN;
	// set to input
	SENSORS_DDR_PORT = SENSORS_DDR_PORT & (uint8_t)~sensor_mask;
	// activate pullup if necessary
	#ifdef SENSORS_PULLUP
	SENSORS_PORT |= sensor_mask;
	#endif
}