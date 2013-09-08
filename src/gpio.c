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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "gpio.h"

#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define MMAP_GPIO_OE(var) map[((var)-MMAP_OFFSET+GPIO_OE)/4]
#define MMAP_GPIO_DATAIN(var) map[((var)-MMAP_OFFSET+GPIO_DATAIN)/4]
#define MMAP_GPIO_DATAOUT(var) map[((var)-MMAP_OFFSET+GPIO_DATAOUT)/4]

static volatile uint32_t *map = NULL;
unsigned int delay_loops = 0;

bool calibrate_delay()
{
	struct timespec first, second;
	long nsec;

	/* first run may be uncached in the processor */
	if (clock_gettime(CLOCK_MONOTONIC, &first) != 0
	|| clock_gettime(CLOCK_MONOTONIC, &first) != 0
	|| clock_gettime(CLOCK_MONOTONIC, &first) != 0
	|| clock_gettime(CLOCK_MONOTONIC, &second) != 0) {
		perror("Error calibrating delay");
		return false;
	}
	nsec = (second.tv_sec - first.tv_sec) * 1000000000
	+ (second.tv_nsec - first.tv_nsec);
	if (nsec <= 0) {
		printf("Zero or less nsec elapsed while calibrating delay!\n");
		return false;
	}
	delay_loops = 15 / nsec;	/* 33 MHz chip, half cycle is 15 ns */
	if ((15 % nsec) != 0)
		delay_loops++;
	if (delay_loops == 0)
		delay_loops = 1;
	return true;
}

void delay()
{
	unsigned int i;
	struct timespec stub;
	for (i = 0; i < delay_loops; i++)
		clock_gettime(CLOCK_MONOTONIC, &stub);
}

bool mmap_init()
{
	int fd;
	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		perror("Unable to open /dev/mem");
		return false;
	}
	map = (uint32_t *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, MMAP_OFFSET);
	if (map == MAP_FAILED) {
		close(fd);
		perror("Unable to map /dev/mem");
		return false;
	}
	return true;
}

void mmap_cleanup()
{
	if (map == NULL)
		return;
	if (munmap((void *)map, MMAP_SIZE) != 0)
		perror("Error unmapping");
	map = NULL;
}

bool init_pin(PIN *pin)
{
	FILE *fp = NULL;
	if ((fp = fopen(GPIO_EXPORT, "w")) == NULL) {
		perror("Cannot set pin to gpio mode");
		return false;
	}
	if (fprintf(fp, "%i", pin->gpio) <= 0) {
		printf("Cannot write pin mode\n");
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

static bool init_block(PIN *block, int n)
{
	int i;
	for (i = 0; i < n; i++)
		if (!init_pin(&block[i]))
			return false;
	return true;
}

bool init_nibble(PIN *nibble)
{
	return init_block(nibble, 4);
}

bool reset_pin(PIN *pin)
{
	FILE *fp = NULL;
	if ((fp = fopen(GPIO_UNEXPORT, "w")) == NULL) {
		perror("Cannot reset pin mode");
		return false;
	}
	if (fprintf(fp, "%i", pin->gpio) <= 0) {
		printf("Cannot write pin mode\n");
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

static bool reset_block(PIN *block, int n)
{
	int i;
	for (i = 0; i < n; i++)
		if (!reset_pin(&block[i]))
			return false;
	return true;
}

bool reset_nibble(PIN *nibble)
{
	return reset_block(nibble, 4);
}

void pin_set_dir(PIN *pin, uint8_t dir)
{
	if (map == NULL)
		return;
	if (dir == INPUT)
		MMAP_GPIO_OE(pin->gpio_bank) |= 1 << pin->bank_id;
	else
		MMAP_GPIO_OE(pin->gpio_bank) &= ~(1 << pin->bank_id);
}

static void block_set_dir(PIN *block, int n, uint8_t dir)
{
	if (map == NULL)
		return;
	if (dir == INPUT)
		MMAP_GPIO_OE(block[0].gpio_bank) |= ((1 << n) - 1) << block[0].bank_id;
	else
		MMAP_GPIO_OE(block[0].gpio_bank) &= ~(((1 << n) - 1) << block[0].bank_id);
}

void nibble_set_dir(PIN *nibble, uint8_t dir)
{
	block_set_dir(nibble, 4, dir);
}

int pin_read(PIN *pin)
{
	return (MMAP_GPIO_DATAIN(pin->gpio_bank) >> pin->bank_id) & 1;
}

void pin_write(PIN *pin, uint8_t value)
{
	if (value == HIGH)
		MMAP_GPIO_DATAOUT(pin->gpio_bank) |= 1 << pin->bank_id;
	else
		MMAP_GPIO_DATAOUT(pin->gpio_bank) &= ~(1 << pin->bank_id);
}

static uint32_t block_read(PIN *block, int n)
{
	return (MMAP_GPIO_DATAIN(block[0].gpio_bank) >> block[0].bank_id) & ((1 << n) - 1);
}

static void block_write(PIN *block, int n, uint32_t data)
{
	MMAP_GPIO_DATAOUT(block[0].gpio_bank) = (MMAP_GPIO_DATAOUT(block[0].gpio_bank)
		& ~(((1 << n)-1) << block[0].bank_id)) | (data << block[0].bank_id);
}

uint8_t nibble_read(PIN *nibble)
{
	return block_read(nibble, 4);
}

void nibble_write(PIN *nibble, uint8_t data)
{
	block_write(nibble, 4, data);
}

void clock_delay(PIN *clock, uint8_t value)
{
	pin_write(clock, value);
	delay();
}

void clock_low(PIN *clock)
{
	clock_delay(clock, LOW);
}

void clock_high(PIN *clock)
{
	clock_delay(clock, HIGH);
}

void clock_cycle(PIN *clock)
{
	clock_low(clock);
	clock_high(clock);
}
