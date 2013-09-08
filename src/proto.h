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
#ifndef PROTO_H_
#define PROTO_H_

#include "fwh.h"
#include "lpc.h"

typedef struct PROTO_s PROTO;
typedef enum MODE_e { MODE_FWH, MODE_LPC } MODE;
typedef struct PROTO_s {
	MODE mode;
	union proto_u {
		FWH fwh;
		LPC lpc;
	} ctx;
} PROTO;

bool proto_init(PROTO *proto, MODE mode, PIN *nibble, PIN *clock, PIN *frame_, PIN *init_);
bool proto_cleanup(PROTO *proto);
bool proto_read_address(PROTO *proto, uint32_t addr, uint8_t *byte);
bool proto_write_address(PROTO *proto, uint32_t addr, uint8_t byte);

#endif /* PROTO_H_ */
