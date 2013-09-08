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
#include "proto.h"

bool proto_init(PROTO *proto, MODE mode, PIN *nibble, PIN *clock, PIN *frame_, PIN *init_)
{
	proto->mode = mode;
	switch (mode) {
	case MODE_FWH:
		proto->ctx.fwh.nibble = nibble;
		proto->ctx.fwh.clock = clock;
		proto->ctx.fwh.fwh4 = frame_;
		proto->ctx.fwh.init_ = init_;
		return fwh_init(&proto->ctx.fwh);
	case MODE_LPC:
		proto->ctx.lpc.nibble = nibble;
		proto->ctx.lpc.clock = clock;
		proto->ctx.lpc.lframe_ = frame_;
		proto->ctx.lpc.init_ = init_;
		return lpc_init(&proto->ctx.lpc);
	default:
		break;
	}
	return false;
}

bool proto_cleanup(PROTO *proto)
{
	switch (proto->mode) {
	case MODE_FWH:
		fwh_cleanup(&proto->ctx.fwh);
		return true;
	case MODE_LPC:
		lpc_cleanup(&proto->ctx.lpc);
		return true;
	default:
		break;
	}
	return false;
}

bool proto_read_address(PROTO *proto, uint32_t addr, uint8_t *byte)
{
	switch (proto->mode) {
	case MODE_FWH:
		return fwh_read_address(&proto->ctx.fwh, addr, byte);
	case MODE_LPC:
		return lpc_read_address(&proto->ctx.lpc, addr, byte);
	default:
		break;
	}
	return false;
}

bool proto_write_address(PROTO *proto, uint32_t addr, uint8_t byte)
{
	switch (proto->mode) {
	case MODE_FWH:
		return fwh_write_address(&proto->ctx.fwh, addr, byte);
	case MODE_LPC:
		return lpc_write_address(&proto->ctx.lpc, addr, byte);
	default:
		break;
	}
	return false;
}
