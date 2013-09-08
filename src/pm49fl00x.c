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
#include <stdio.h>
#include "pm49fl00x.h"

bool pm49fl00x_read_manuf_id(PROTO *proto, uint8_t *manuf)
{
	return proto_read_address(proto, PM49FL00X_MANUF_ID_ADDR, manuf);
}

bool pm49fl00x_read_device_id(PROTO *proto, uint8_t *dev)
{
	return proto_read_address(proto, PM49FL00X_DEVICE_ID_ADDR, dev);
}

bool pm49fl004_unlock_block(PROTO *proto, unsigned int block_num)
{
	uint32_t blr;
	uint8_t byte;

	if (block_num >= PM49FL004_BLOCKS)
		return false;
	blr = PM49FL004_BLOCK_LOCK_REG0 + block_num * PM49FL004_BLOCK_LOCK_OFFSET;
	if (!proto_read_address(proto, blr, &byte)
	|| (byte & PM49FL00X_BLOCK_LOCK_LOCK_DOWN) != 0
	|| !proto_write_address(proto, blr, byte & ~(PM49FL00X_BLOCK_LOCK_READ_LOCK
	| PM49FL00X_BLOCK_LOCK_WRITE_LOCK))
	|| !proto_read_address(proto, blr, &byte)
	|| (byte & (PM49FL00X_BLOCK_LOCK_READ_LOCK
	| PM49FL00X_BLOCK_LOCK_WRITE_LOCK)) != 0)
		return false;
	return true;
}

bool pm49fl004_erase_block(PROTO *proto, unsigned int block_num)
{
	uint32_t addr, offset;
	uint8_t byte;
	struct timespec ts, now;

	if (block_num >= PM49FL004_BLOCKS)
		return false;
	addr = PM49FL004_MEM_BASE_ADDR + block_num * PM49FL004_BLOCK_SIZE;
	/* JEDEC */
	if (!proto_write_address(proto, 0xffff5555, 0xaa)
	|| !proto_write_address(proto, 0xffff2aaa, 0x55)
	|| !proto_write_address(proto, 0xffff5555, 0x80)
	|| !proto_write_address(proto, 0xffff5555, 0xaa)
	|| !proto_write_address(proto, 0xffff2aaa, 0x55)
	|| !proto_write_address(proto, addr, 0x50)
	|| clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		return false;
	do {
		for (offset = 0; offset < PM49FL004_BLOCK_SIZE; offset++) {
			if (!proto_read_address(proto, addr + offset, &byte))
				return false;
			if (byte != 0xff)
				break;
		}
		if (clock_gettime(CLOCK_MONOTONIC, &now) != 0
		|| (now.tv_sec - ts.tv_sec) * 1000000000LL
		+ now.tv_nsec - ts.tv_nsec > 99000000000LL)
			return false;
	} while (byte != 0xff);
	return true;
}

bool pm49fl004_program_block(PROTO *proto, unsigned int block_num, uint8_t *block)
{
	uint32_t addr, offset;
	uint8_t byte;
	struct timespec ts, now;

	if (block_num >= PM49FL004_BLOCKS)
		return false;
	addr = PM49FL004_MEM_BASE_ADDR + block_num * PM49FL004_BLOCK_SIZE;
	for (offset = 0; offset < PM49FL004_BLOCK_SIZE; offset++) {
		/* JEDEC */
		if (!proto_write_address(proto, 0xffff5555, 0xaa)
		|| !proto_write_address(proto, 0xffff2aaa, 0x55)
		|| !proto_write_address(proto, 0xffff5555, 0xa0)
		|| !proto_write_address(proto, addr + offset, block[offset])
		|| clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
			return false;
		do {
			if (!proto_read_address(proto, addr + offset, &byte)
			|| clock_gettime(CLOCK_MONOTONIC, &now) != 0
			|| (now.tv_sec - ts.tv_sec) * 1000000000LL
			+ now.tv_nsec - ts.tv_nsec > 99000000000LL)
				return false;
		} while (byte != block[offset]);
	}
	return true;
}

bool pm49fl004_rewrite_chip(PROTO *proto, uint8_t *prog)
{
	unsigned int block_num;

	for (block_num = 0; block_num < PM49FL004_BLOCKS; block_num++) {
		printf("Unlocking block %u\n", block_num);
		if (!pm49fl004_unlock_block(proto, block_num))
			return false;
		printf("Erasing block %u\n", block_num);
		if (!pm49fl004_erase_block(proto, block_num))
			return false;
		printf("Programming block %u\n", block_num);
		if (!pm49fl004_program_block(proto, block_num, prog
		+ block_num * PM49FL004_BLOCK_SIZE))
			return false;
	}
	return true;
}

bool pm49fl004_dump_chip(PROTO *proto, uint8_t *dump)
{
	unsigned int block_num;
	uint32_t addr, offset;

	for (block_num = 0; block_num < PM49FL004_BLOCKS; block_num++) {
		printf("Dumping block %u\n", block_num);
		addr = PM49FL004_MEM_BASE_ADDR + block_num * PM49FL004_BLOCK_SIZE;
		for (offset = 0; offset < PM49FL004_BLOCK_SIZE; offset++, dump++)
			if (!proto_read_address(proto, addr + offset, dump))
				return false;
	}
	return true;
}
