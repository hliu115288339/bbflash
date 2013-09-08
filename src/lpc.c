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
#include "lpc.h"

#define LPC_START 0b0000
#define LPC_CYCTYPE_READ 0b0100
#define LPC_CYCTYPE_WRITE 0b0110

bool lpc_init(LPC *lpc)
{
	return nibble_init(lpc->init_, lpc->clock, lpc->lframe_, lpc->nibble);
}

void lpc_cleanup(LPC *lpc)
{
	nibble_cleanup(lpc->init_, lpc->clock, lpc->lframe_, lpc->nibble);
}

static void lpc_start(LPC *lpc)
{
	nibble_start(lpc->clock, lpc->lframe_, lpc->nibble, LPC_START);
}

static void lpc_nibble_write(LPC *lpc, uint8_t value)
{
	clocked_nibble_write(lpc->clock, lpc->nibble, value);
}

static void lpc_send_addr(LPC *lpc, uint32_t addr)
{
	int i;
	for (i = 28; i >= 0; i -= 4)
		lpc_nibble_write(lpc, (addr >> i) & 0xf);
}

bool lpc_read_address(LPC *lpc, uint32_t addr, uint8_t *byte)
{
	lpc_start(lpc);
	lpc_nibble_write(lpc, LPC_CYCTYPE_READ);
	lpc_send_addr(lpc, addr);
	nibble_set_dir(lpc->nibble, INPUT);
	clock_cycle(lpc->clock);
	if (!nibble_ready_sync(lpc->clock, lpc->nibble))
		return false;
	*byte = byte_read(lpc->clock, lpc->nibble);
	clock_cycle(lpc->clock);
	clock_cycle(lpc->clock);
	clock_cycle(lpc->clock);
	return true;
}

bool lpc_write_address(LPC *lpc, uint32_t addr, uint8_t byte)
{
	lpc_start(lpc);
	lpc_nibble_write(lpc, LPC_CYCTYPE_WRITE);
	lpc_send_addr(lpc, addr);
	byte_write(lpc->clock, lpc->nibble, byte);
	nibble_set_dir(lpc->nibble, INPUT);
	clock_cycle(lpc->clock);
	clock_cycle(lpc->clock);
	if (!nibble_ready_sync(lpc->clock, lpc->nibble))
		return false;
	clock_cycle(lpc->clock);
//	clock_cycle(lpc->clock);
	return true;
}
