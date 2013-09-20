/*
 * ET6220 write to display utility
 * 
 * Copyright (c) 2013 Jesus Trujillo <elyeyus@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
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
#include "lib/segment_parser.h"

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

static uint8_t brightness = 7;
static const char *message = "0:00:00";

static void print_usage(const char *prog)
{
	printf("Usage: %s [-Dsdb] -m \n", prog);
	puts("  -D --device   device to use (default /dev/spidev0.0)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bright   brightness 0 to 7\n"
	     "  -m --message  message string\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  required_argument, 0, 'D' },
			{ "speed",   required_argument, 0, 's' },
			{ "delay",   required_argument, 0, 'd' },
			{ "bright",  required_argument, 0, 'b' },
			{ "message", required_argument, 0, 'm' },		
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:m:", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			brightness = atoi(optarg);
			break;
		case 'm':
			message = optarg;
			break;
		default:
			printf("%c\n", c);
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

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

	et6220_init(&dev, DISPLAY_8SEGMENTS);

	et6220_write_mode(&dev, DISPLAY_ADDRESS_AUTO_INCREMENT);

	
	uint8_t char_pos = 0;
	uint8_t segment_pos = 1;
	et6220_display_data data = {
		.g1 = 0,
		.g2 = 0,
		.g3 = 0,
		.g4 = 0,
		.g5 = 0
	};
	parse(message, &char_pos, &segment_pos, &data);

	et6220_send_data(&dev, &data);

	uint8_t buf[1];
	buf[0] = cmd1_display_mode(DISPLAY_8SEGMENTS);
	et6220_command(&dev, buf, buf, 1);

	buf[0] = cmd4_display_control(DISPLAY_ON,brightness);
	et6220_command(&dev, buf, buf, 1);

	close(fd);

	return ret;
}