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
#ifndef NIBBLE_H_
#define NIBBLE_H_

#include "gpio.h"

bool nibble_init(PIN *init_, PIN *clock, PIN *frame_, PIN *nibble);
void nibble_cleanup(PIN *init_, PIN *clock, PIN *frame_, PIN *nibble);
void clocked_nibble_write(PIN *clock, PIN *nibble, uint8_t value);
uint8_t clocked_nibble_read(PIN *clock, PIN *nibble);
void nibble_start(PIN *clock, PIN *frame_, PIN *nibble, uint8_t start);
bool nibble_ready_sync(PIN *clock, PIN *nibble);
uint8_t byte_read(PIN *clock, PIN *nibble);
void byte_write(PIN *clock, PIN *nibble, uint8_t byte);

#endif /* NIBBLE_H_ */
