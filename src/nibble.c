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
#include <time.h>
#include "gpio.h"

bool nibble_init(PIN *init_, PIN *clock, PIN *frame_, PIN *nibble)
{
	unsigned int i;

	if (!reset_pin(clock) || !reset_pin(frame_)
	|| !reset_nibble(nibble) || !init_pin(clock)
	|| !init_pin(frame_) || !init_nibble(nibble)
	|| !init_pin(init_) || !calibrate_delay())
		return false;

	pin_set_dir(init_, OUTPUT);
	pin_write(init_, LOW);

	pin_set_dir(clock, OUTPUT);
	pin_write(clock, HIGH);

	pin_set_dir(frame_, OUTPUT);
	pin_write(frame_, HIGH);

	nibble_set_dir(nibble, OUTPUT);
	nibble_write(nibble, 0);

	for (i = 0; i < 24; i++)
		clock_cycle(clock);
	pin_write(init_, HIGH);
	for (i = 0; i < 42; i++)
		clock_cycle(clock);

	return true;
}

void nibble_cleanup(PIN *init_, PIN *clock, PIN *frame_, PIN *nibble)
{
	pin_set_dir(clock, INPUT);
	pin_set_dir(frame_, INPUT);
	nibble_set_dir(nibble, INPUT);
	reset_pin(clock);
	reset_pin(frame_);
	reset_nibble(nibble);
	reset_pin(init_);
}

void clocked_nibble_write(PIN *clock, PIN *nibble, uint8_t value)
{
	clock_low(clock);
	nibble_write(nibble, value);
	delay();
	clock_high(clock);
	delay();
}

uint8_t clocked_nibble_read(PIN *clock, PIN *nibble)
{
	clock_cycle(clock);
	return nibble_read(nibble);
}

void nibble_start(PIN *clock, PIN *frame_, PIN *nibble, uint8_t start)
{
	pin_write(frame_, HIGH);
	nibble_set_dir(nibble, OUTPUT);
	clock_high(clock);
	pin_write(frame_, LOW);
	nibble_write(nibble, start);
	clock_cycle(clock);
	pin_write(frame_, HIGH);
	delay();
}

bool nibble_ready_sync(PIN *clock, PIN *nibble)
{
	uint8_t nib;
	struct timespec ts, now;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		return false;
	do {
		nib = clocked_nibble_read(clock, nibble);
		if (clock_gettime(CLOCK_MONOTONIC, &now) != 0
		|| (now.tv_sec - ts.tv_sec) * 1000000000
		+ now.tv_nsec - ts.tv_nsec > 1000000000)
			return false;
	} while (nib != 0);
	return true;
}

uint8_t byte_read(PIN *clock, PIN *nibble)
{
	return clocked_nibble_read(clock, nibble)
	| (clocked_nibble_read(clock, nibble) << 4);
}

void byte_write(PIN *clock, PIN *nibble, uint8_t byte)
{
	clocked_nibble_write(clock, nibble, byte & 0xf);
	clocked_nibble_write(clock, nibble, byte >> 4);
}
