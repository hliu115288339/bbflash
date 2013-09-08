/*
	This file is part of bbflash.
	Copyright (C) 2013, Hao Liu and Robert L. Thompson

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdbool.h>
#include <stdint.h>
#include "am335x.h"

#define HIGH ((uint8_t)(1))
#define LOW  ((uint8_t)(0))

bool calibrate_delay();
void delay();

bool mmap_init();
void mmap_cleanup();

bool init_pin(PIN *pin);
bool init_nibble(PIN *nibble);

bool reset_pin(PIN *pin);
bool reset_nibble(PIN *nibble);

void pin_set_dir(PIN *pin, uint8_t dir);
void nibble_set_dir(PIN *nibble, uint8_t dir);

int pin_read(PIN *pin);
void pin_write(PIN *pin, uint8_t value);

uint8_t nibble_read(PIN *nibble);
void nibble_write(PIN *nibble, uint8_t data);

void clock_delay(PIN *clock, uint8_t value);
void clock_low(PIN *clock);
void clock_high(PIN *clock);
void clock_cycle(PIN *clock);

#endif
