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
#include "nibble.h"
#include "fwh.h"

#define FWH_START_READ 0b1101
#define FWH_START_WRITE 0b1110
#define FWH_ABORT 0b1111

bool fwh_init(FWH *fwh)
{
	return nibble_init(fwh->init_, fwh->clock, fwh->fwh4, fwh->nibble);
}

void fwh_cleanup(FWH *fwh)
{
	nibble_cleanup(fwh->init_, fwh->clock, fwh->fwh4, fwh->nibble);
}

static void fwh_nibble_write(FWH *fwh, uint8_t value)
{
	clocked_nibble_write(fwh->clock, fwh->nibble, value);
}

static void fwh_start(FWH *fwh, uint8_t start)
{
	nibble_start(fwh->clock, fwh->fwh4, fwh->nibble, start);
}

static void fwh_send_imaddr(FWH *fwh, uint32_t addr)
{
	int i;
	for (i = 24; i >= 0; i -= 4)
		fwh_nibble_write(fwh, (addr >> i) & 0xf);
}

bool fwh_read_address(FWH *fwh, uint32_t addr, uint8_t *byte)
{
	fwh_start(fwh, FWH_START_READ);
	fwh_nibble_write(fwh, 0);	/* IDSEL hardwired */
	fwh_send_imaddr(fwh, addr);
	fwh_nibble_write(fwh, 0);	/* IMSIZE single byte */
	nibble_set_dir(fwh->nibble, INPUT);
	clock_cycle(fwh->clock);
	if (!nibble_ready_sync(fwh->clock, fwh->nibble))
		return false;
	*byte = byte_read(fwh->clock, fwh->nibble);
	clock_cycle(fwh->clock);
	nibble_set_dir(fwh->nibble, OUTPUT);
	fwh_nibble_write(fwh, 0xf);
	clock_cycle(fwh->clock);
	return true;
}

bool fwh_write_address(FWH *fwh, uint32_t addr, uint8_t byte)
{
	fwh_start(fwh, FWH_START_WRITE);
	fwh_nibble_write(fwh, 0);	/* IDSEL hardwired */
	fwh_send_imaddr(fwh, addr);
	fwh_nibble_write(fwh, 0);	/* IMSIZE single byte */
	byte_write(fwh->clock, fwh->nibble, byte);
	nibble_write(fwh->nibble, 0xf);
	nibble_set_dir(fwh->nibble, INPUT);
	clock_cycle(fwh->clock);
	clock_cycle(fwh->clock);
	if (!nibble_ready_sync(fwh->clock, fwh->nibble))
		return false;
	clock_cycle(fwh->clock);
	return true;
}
