/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <byteswap.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/mman.h>
#include "cxdma.h"

/* ltoh: little to host */
/* htol: little to host */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ltohl(x)       (x)
#define ltohs(x)       (x)
#define htoll(x)       (x)
#define htols(x)       (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ltohl(x)     __bswap_32(x)
#define ltohs(x)     __bswap_16(x)
#define htoll(x)     __bswap_32(x)
#define htols(x)     __bswap_16(x)
#endif

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)

int devinfo(DEVICE *device) //char *device, unsigned int address, void *map_base, void *virt_addr)
{
	int fd;
	uint32_t read_result, writeval;
	int access_width = 'w';
    
	if ((fd = open(device->path, O_RDWR | O_SYNC)) == -1)
		FATAL;
	printf("character device %s opened.\n", device->path);
	fflush(stdout);

	device->map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (device->map_base == (void *)-1)
		FATAL;
	printf("Memory mapped at address %p.\n", device->map_base);
	fflush(stdout);

	device->virt_addr = device->map_base + device->address;
    read_result = *((uint32_t *) device->virt_addr);
    /* swap 32-bit endianess if host is not little-endian */
    read_result = ltohl(read_result);
    
	printf("Memory mapped at address %p.\n", device->virt_addr);
    printf("%p \n", read_result);
    printf("%p \n", (read_result >> 20) & 0xFFF);
    printf("%p \n", (read_result >> 12) & 0xFF);
    
	if (munmap(device->map_base, MAP_SIZE) == -1)
		FATAL;
	close(fd);
	return 0;
}

int openDev(char *device){
    return open(device, O_RDWR | O_SYNC);
}

int closeDev(int fd){
    close(fd);
	return 0;
}

void* getBase(int fd, void* map_base){
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return map_base;
}

uint32_t readDev(void* virt_addr, uint32_t read_result){
    read_result = *((uint32_t *) virt_addr);
    /* swap 32-bit endianess if host is not little-endian */
    read_result = ltohl(read_result);    
    return read_result;
}