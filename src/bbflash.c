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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "pm49fl00x.h"

/* block of 4 contiguous GPIO pins */
#define B4_0 P8_12	/* gpio 44 */
#define B4_1 P8_11	/* gpio 45 */
#define B4_2 P8_16	/* gpio 46 */
#define B4_3 P8_15	/* gpio 47 */
#define CLOCK P8_14	/* gpio 26 */
#define FRAME_ P8_17	/* gpio 27 */
#define INIT_ P8_18	/* gpio 65 */

int open_rom_file(const char *name, off_t expected_size, void **file);
int create_dump_file(const char *name, off_t max_size, void **file);

int main(int argc, char *argv[]) {
	PIN nibble[4] = { B4_0, B4_1, B4_2, B4_3 };
	PIN clock = CLOCK;
	PIN frame_ = FRAME_;
	PIN init_ = INIT_;
	PROTO proto;
	uint8_t manuf_id, device_id;
	int rc, fd = -1;
	void *file = NULL;
	enum OP_E { OP_DUMP, OP_FLASH } op;

	if (argc < 3) {
		printf("%s dump|flash file\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (strcasecmp(argv[1], "dump") == 0)
		op = OP_DUMP;
	else
		if (strcasecmp(argv[1], "flash") == 0)
			op = OP_FLASH;
		else {
			printf("%s dump|flash file\n", argv[0]);
			return EXIT_FAILURE;
		}

	if (!mmap_init() || !proto_init(&proto, MODE_FWH, nibble, &clock, &frame_, &init_))
		goto error;

	if (op == OP_FLASH)
		fd = open_rom_file(argv[2], PM49FL004_SIZE, &file);
	else	/* OP_DUMP */
		fd = create_dump_file(argv[2], PM49FL004_SIZE, &file);
	if (fd < 0)
		goto error;
	if (!pm49fl00x_read_manuf_id(&proto, &manuf_id)
	|| !pm49fl00x_read_device_id(&proto, &device_id))
		goto error;
	printf("Manufacturer ID: %02x\n", manuf_id);
	printf("Device ID: %02x\n", device_id);
	if (manuf_id != PM49FL00X_MANUF_ID || device_id != PM49FL004_DEVICE_ID) {
		printf("Incorrect IDs for PM49FL004 flash chip!\n");
		goto error;
	}
	if (op == OP_FLASH) {
		if (!pm49fl004_rewrite_chip(&proto, file)) {
			printf("Failed!\n");
			goto error;
		}
	} else	/* OP_FLASH */
		if (!pm49fl004_dump_chip(&proto, file)) {
			printf("Failed!\n");
			goto error;
		}
	printf("Success!\n");
	rc = EXIT_SUCCESS;
cleanup:
	proto_cleanup(&proto);
	mmap_cleanup();
	if (file != NULL) {
		msync(file, PM49FL004_SIZE, MS_SYNC);
		munmap(file, PM49FL004_SIZE);
	}
	if (fd >= 0)
		close(fd);
	return rc;
error:
	rc = EXIT_FAILURE;
	goto cleanup;
}

int open_rom_file(const char *name, off_t expected_size, void **file)
{
	int fd;
	struct stat st;

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		perror("open()");
		return -1;
	}
	if (fstat(fd, &st) != 0) {
		perror("fstat()");
		close(fd);
		return -1;
	}
	if (st.st_size != expected_size) {
		printf("File size does not match chip size!\n");
		close(fd);
		return -1;
	}
	*file = mmap(NULL, PM49FL004_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	if (*file == MAP_FAILED) {
		perror("mmap()");
		close(fd);
		return -1;
	}
	return fd;
}

int create_dump_file(const char *name, off_t max_size, void **file)
{
	int fd;

	fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0) {
		perror("open()");
		return -1;
	}
	if (ftruncate(fd, max_size) != 0) {
		perror("ftruncate()");
		close(fd);
		return -1;
	}
	*file = mmap(NULL, PM49FL004_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (*file == MAP_FAILED) {
		perror("mmap()");
		close(fd);
		return -1;
	}
	return fd;
}
