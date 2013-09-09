/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
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

#define DEBUG 1

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

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

/*
 * ASCII TO SEGMENT
 */

static uint8_t number_to_segment[10] = {
	0b11111100, //0
	0b01100000, //1
	0b11011010, //2
	0b11110010, //3
	0b01100110, //4
	0b10110100, //5
	0b10111110, //6
	0b11100000, //7
	0b11111110, //8
	0b11100110, //9
};

// TODO Letters
/*
 * A,b,C,c,d,E,e,F,G,H,I,J,j,L,O,P,S,T,U,u,Y,=,-,º,[,],",'
 */

/************/
/* Commands */
/************/

/* 
 * CMD1 - Display mode setting command
 *       b7 b6 b5 b4 b3 b2 b1 b0
 *        0  0  -  -  -  -  x  x
 *  xx: 00 9 segments x 4 groups
 *      01 8 segments x 5 groups
 *      10 7 segments x 6 groups
 *      11 6 segments x 7 groups
 */
#define DISPLAY_9SEGMENTS 0
#define DISPLAY_8SEGMENTS 1
#define DISPLAY_7SEGMENTS 2
#define DISPLAY_6SEGMENTS 3
static uint8_t cmd1_display_mode (uint8_t segments)
{
	if(segments > 5 && segments < 10) {
		return (9-segments);
	}
	return 0xFF;
}

/*
 * CMD2 - Data setting command
 *      b7 b6 b5 b4 b3 b2 b1 b0
 *       0  1  -  -  x  y  z  z
 *  x: 0 = Normal mode
 *     1 = Test mode
 *  y: 0 = Increment address after data is written
 *     1 = Keep address unchanged
 *  zz: 00 = Write data to the display
 *      01 = Read key scan data
 */
#define DISPLAY_MODE_NORMAL 0
#define DISPLAY_MODE_TEST 1
#define DISPLAY_ADDRESS_AUTO_INCREMENT 0
#define DISPLAY_ADDRESS_UNCHANGED 1
#define DISPLAY_WRITE 0
#define DISPLAY_READ_KEYS 1
static uint8_t cmd2_data_setting (uint8_t mode, uint8_t address_increment, uint8_t read_mode)
{
	uint8_t ret = 0x40;
	// Setting display mode value
	ret = ret|((mode&1)<<3);
	// Setting address increment mode
	ret = ret|((address_increment&1)<<2);
	// Setting write or read mode
	ret = ret|(read_mode&1);

	return ret;
}

/*
 * CMD3 - Address setting command
 *      b7 b6 b5 b4 b3 b2 b1 b0
 *       1  1  -  -  x  x  x  x
 * xxxx: Address position (0x00 -> 0x0D)
 */
static uint8_t cmd3_set_address (uint8_t address)
{
	uint8_t ret = 0xC0;
	if (address >= 0x00 && address <= 0x0D) {
		ret = ret|(address&0x0F);
	}
	return ret;
}

/*
 * CMD4 - Display control command
 *     b7 b6 b5 b4 b3 b2 b1 b0
 *      1  0  -  -  x  y  y  y
 *  x: 0 = Display off (key scan continues)
 *     1 = Display on
 *  yyy: Dimmer level setting 0 to 8 levels
 */
#define DISPLAY_OFF 0
#define DISPLAY_ON 1
static uint8_t cmd4_display_control (uint8_t on, uint8_t brightness)
{
	uint8_t ret = 0x80;
	ret = ret|((on&1)<<3);
	ret = ret|(brightness&0x07);
	return ret;
}

uint8_t lookup[16] = {
   0x0, 0x8, 0x4, 0xC,
   0x2, 0xA, 0x6, 0xE,
   0x1, 0x9, 0x5, 0xD,
   0x3, 0xB, 0x7, 0xF };

uint8_t flip( uint8_t n )
{
   //This should be just as fast and it is easier to understand.
   //return (lookup[n%16] << 4) | lookup[n/16];
   return (lookup[n&0x0F] << 4) | lookup[n>>4];
}

static uint8_t command(int fd, uint8_t tx[], uint8_t *rx, uint8_t size)
{
	int ret;

	uint8_t i;
	for(i=0;i<size;i++) {
		tx[i] = flip(tx[i]);
	}

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = size,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror("Can't send api message");
		return ret;
	}

	#ifdef DEBUG
	for (ret = 0; ret < size; ret++) {
		if(!(ret%8)) puts("");
		printf("%.2X ", rx[ret]);
	}
	#endif

	return ret;
}

static void et6220_init(int fd, uint8_t segments)
{
	
	uint8_t rd_buff[20];

	uint8_t cmd2_write[1];
	cmd2_write[0] = cmd2_data_setting(DISPLAY_MODE_NORMAL,DISPLAY_ADDRESS_AUTO_INCREMENT,DISPLAY_WRITE);
	
	printf("Starting ET6220 device...\n");
	printf("CMD2 %.2X\n",cmd2_write[0]);
	command(fd, cmd2_write, rd_buff, 1);

	uint8_t cmd3_clear_ram[1];
	cmd3_clear_ram[0] = cmd3_set_address(0x00);
	printf("CMD3 %.2X\n",cmd3_clear_ram[0]);
	command(fd, cmd3_clear_ram, rd_buff, 1);

	uint8_t cmd1_setup_mode[1];
	cmd1_setup_mode[0] = cmd1_display_mode(segments);
	printf("CMD1 %.2X\n",cmd1_setup_mode[0]);
	command(fd, cmd1_setup_mode, rd_buff, 1);

	uint8_t cmd4_display_off[1];
	cmd4_display_off[0] = cmd4_display_control(DISPLAY_OFF,7);
	printf("CMD4 %.2X\n", cmd4_display_off[0]);
	command(fd, cmd4_display_off, rd_buff, 1);

	printf("CMD1 %.2X\n", cmd1_setup_mode[0]);
	command(fd, cmd1_setup_mode, rd_buff, 1);

	uint8_t cmd4_display_on[1];
	cmd4_display_on[0] = cmd4_display_control(DISPLAY_ON,7);
	printf("CMD4 %.2X", cmd4_display_on[0]);
	command(fd, cmd4_display_on, rd_buff, 1);

}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev0.0)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

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
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
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

	et6220_init(fd,DISPLAY_8SEGMENTS);

	uint8_t set_write_mode[1];
	set_write_mode[0] = cmd2_data_setting(DISPLAY_MODE_NORMAL, DISPLAY_ADDRESS_AUTO_INCREMENT, DISPLAY_WRITE);
	command(fd, set_write_mode, set_write_mode, 1);

	uint8_t display_data[15] = {
		0x00,0xFF,0xFF,0xFF,
		0x00,0xFF,0x00,0xFF,
		0x00,0xFF,0x00,0xFF,
		0x00,0xFF,0x00};
	display_data[1] = 0xFF;//number_to_segment[8];
	display_data[3] = 0x00;//number_to_segment[2];
	display_data[5] = 0x00;//number_to_segment[3];
	display_data[7] = 0x00;//number_to_segment[4];
	display_data[9] = 0x00;//number_to_segment[5];
	display_data[0] = cmd3_set_address(0x00);
	command(fd, display_data, display_data, 15);

	uint8_t buf[1];
	buf[0] = cmd1_display_mode(DISPLAY_8SEGMENTS);
	command(fd, buf, buf, 1);

	buf[0] = cmd4_display_control(DISPLAY_ON,0);
	command(fd, buf, buf, 1);

	close(fd);

	return ret;
}
