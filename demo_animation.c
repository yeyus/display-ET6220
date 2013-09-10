/*
 * ET6220 animation demo
 *
 * Copyright (c) 2013  Jesus Trujillo <elyeyus@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 */

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "lib/et6220.h"

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 200000;
static uint16_t delay;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	/* ET6220 CODE */
	spi_et6220_device dev = {
		.spi_fd = fd,
		.spi_delay = delay,
		.spi_speed_hz = speed
	};

	uint8_t flowpos = 0;
	uint8_t bpos = 0;

	uint16_t flow[5] = {
		SEGMENT_A,
		SEGMENT_G,
		SEGMENT_D,
		SEGMENT_G,
		SEGMENT_A
	};

	et6220_init(&dev, DISPLAY_8SEGMENTS);

	et6220_write_mode(&dev, DISPLAY_ADDRESS_AUTO_INCREMENT);

	uint8_t buf[1];
	while(1) {
		
		et6220_display_data data = {
			.g1 = flow[flowpos%5],
			.g2 = flow[(flowpos+1)%5],
			.g3 = flow[(flowpos+2)%5],
			.g4 = flow[(flowpos+3)%5],
			.g5 = flow[(flowpos+4)%5]
		};
		et6220_send_data(&dev, &data);		
		
		buf[0] = cmd1_display_mode(DISPLAY_8SEGMENTS);
		et6220_command(&dev, buf, buf, 1);

		buf[0] = cmd4_display_control(DISPLAY_ON,bpos);
		et6220_command(&dev, buf, buf, 1);

		// increments
		flowpos = ((flowpos+1)%5);
		bpos = ((bpos+1)%8);

		// wait
		sleep(1);
	}

	close(fd);

	return ret;
}