/*
 * ET6220 Segment display library
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

/* CMD1 constants */
#define DISPLAY_9SEGMENTS 9
#define DISPLAY_8SEGMENTS 8
#define DISPLAY_7SEGMENTS 7
#define DISPLAY_6SEGMENTS 6

/* CMD2 constants */
#define DISPLAY_MODE_NORMAL 0
#define DISPLAY_MODE_TEST 1
#define DISPLAY_ADDRESS_AUTO_INCREMENT 0
#define DISPLAY_ADDRESS_UNCHANGED 1
#define DISPLAY_WRITE 0
#define DISPLAY_READ_KEYS 1

/* CMD3 has no constants */

/* CMD4 constants */
#define DISPLAY_OFF 0
#define DISPLAY_ON 1

/* Segments byte

       a
    *******      d=0x0100
   *       *     c=0x0200
 f *       * b   b=0x0400
   *   g   *     a=0x0800
    *******      e=0x1000
   *       *     f=0x2000
 e *       * c   g=0x0008
   *       *     doubledot=0x0010 (only on group 3 and 5)
    *******
       d
*/

#define SEGMENT_A 0x0800
#define SEGMENT_B 0x0400
#define SEGMENT_C 0x0200
#define SEGMENT_D 0x0100
#define SEGMENT_E 0x1000
#define SEGMENT_F 0x2000
#define SEGMENT_G 0x0008
#define SEGMENT_DOT 0x0010

/* On the segment data array high would be data[1] and low data[2] for digit 1 */
#define SEGMENT_BYTE_HIGH(x) ((x&0xFF00)>>8)
#define SEGMENT_BYTE_LOW(x) (x&0x00FF)

/* ET6220 struct */
typedef struct {
	uint32_t spi_fd;
	uint32_t spi_delay;
	uint32_t spi_speed_hz;	
} spi_et6220_device;

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
uint8_t cmd1_display_mode (uint8_t segments);

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
uint8_t cmd2_data_setting (uint8_t mode, uint8_t address_increment, uint8_t read_mode);

/*
 * CMD3 - Address setting command
 *      b7 b6 b5 b4 b3 b2 b1 b0
 *       1  1  -  -  x  x  x  x
 * xxxx: Address position (0x00 -> 0x0D)
 */
uint8_t cmd3_set_address (uint8_t address);

/*
 * CMD4 - Display control command
 *     b7 b6 b5 b4 b3 b2 b1 b0
 *      1  0  -  -  x  y  y  y
 *  x: 0 = Display off (key scan continues)
 *     1 = Display on
 *  yyy: Dimmer level setting 0 to 8 levels
 */
uint8_t cmd4_display_control (uint8_t on, uint8_t brightness);

/* Display handling */

void et6220_init(spi_et6220_device *dev, uint8_t segments);
uint8_t et6220_command(spi_et6220_device *dev, uint8_t tx[], uint8_t *rx, uint8_t size);

