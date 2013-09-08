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
#ifndef PM49FL00X_H_
#define PM49FL00X_H_

#include "proto.h"

#define PM49FL00X_MANUF_ID_ADDR 0xffbc0000
#define PM49FL00X_DEVICE_ID_ADDR 0xffbc0001
#define PM49FL00X_MANUF_ID 0x9d
#define PM49FL002_DEVICE_ID 0x6d
#define PM49FL004_DEVICE_ID 0x6e

#define PM49FL00X_BLOCK_LOCK_READ_LOCK 0x04
#define PM49FL00X_BLOCK_LOCK_LOCK_DOWN 0x02
#define PM49FL00X_BLOCK_LOCK_WRITE_LOCK 0x01

#define PM49FL004_SIZE 524288
#define PM49FL004_BLOCKS 8
#define PM49FL004_MEM_BASE_ADDR 0xfff80000
#define PM49FL004_BLOCK_SIZE 0x10000

#define PM49FL004_BLOCK_LOCK_REG0 0xffb80002
#define PM49FL004_BLOCK_LOCK_REG7 0xffbf0002
#define PM49FL004_BLOCK_LOCK_OFFSET 0x10000

bool pm49fl00x_read_manuf_id(PROTO *proto, uint8_t *manuf);
bool pm49fl00x_read_device_id(PROTO *proto, uint8_t *dev);
bool pm49fl004_unlock_block(PROTO *proto, unsigned int block_num);
bool pm49fl004_erase_block(PROTO *proto, unsigned int block_num);
bool pm49fl004_program_block(PROTO *proto, unsigned int block_num, uint8_t *block);
bool pm49fl004_rewrite_chip(PROTO *proto, uint8_t *prog);
bool pm49fl004_dump_chip(PROTO *proto, uint8_t *dump);

#endif /* PM49FL00X_H_ */
