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

/* MSB first to LSB first translation */
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

/* Commands */

uint8_t cmd1_display_mode (uint8_t segments)
{
	if(segments > 5 && segments < 10) {
		return (9-segments);
	}
	return 0xFF;
}

uint8_t cmd2_data_setting (uint8_t mode, uint8_t address_increment, uint8_t read_mode)
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

uint8_t cmd3_set_address (uint8_t address)
{
	uint8_t ret = 0xC0;
	if (address >= 0x00 && address <= 0x0D) {
		ret = ret|(address&0x0F);
	}
	return ret;
}

uint8_t cmd4_display_control (uint8_t on, uint8_t brightness)
{
	uint8_t ret = 0x80;
	ret = ret|((on&1)<<3);
	ret = ret|(brightness&0x07);
	return ret;
}

/* Device init and high level primitives */
void et6220_init(spi_et6220_device *dev, uint8_t segments)
{
	
	uint8_t rd_buff[20];

	uint8_t cmd2_write[1];
	cmd2_write[0] = cmd2_data_setting(DISPLAY_MODE_NORMAL,DISPLAY_ADDRESS_AUTO_INCREMENT,DISPLAY_WRITE);
	
	printf("Starting ET6220 device...\n");
	printf("CMD2 %.2X\n",cmd2_write[0]);
	et6220_command(dev->spi_fd, cmd2_write, rd_buff, 1);

	uint8_t cmd3_clear_ram[1];
	cmd3_clear_ram[0] = cmd3_set_address(0x00);
	printf("CMD3 %.2X\n",cmd3_clear_ram[0]);
	et6220_command(dev->spi_fd, cmd3_clear_ram, rd_buff, 1);

	uint8_t cmd1_setup_mode[1];
	cmd1_setup_mode[0] = cmd1_display_mode(segments);
	printf("CMD1 %.2X\n",cmd1_setup_mode[0]);
	et6220_command(dev->spi_fd, cmd1_setup_mode, rd_buff, 1);

	uint8_t cmd4_display_off[1];
	cmd4_display_off[0] = cmd4_display_control(DISPLAY_OFF,7);
	printf("CMD4 %.2X\n", cmd4_display_off[0]);
	et6220_command(dev->spi_fd, cmd4_display_off, rd_buff, 1);

	printf("CMD1 %.2X\n", cmd1_setup_mode[0]);
	et6220_command(dev->spi_fd, cmd1_setup_mode, rd_buff, 1);

	uint8_t cmd4_display_on[1];
	cmd4_display_on[0] = cmd4_display_control(DISPLAY_ON,7);
	printf("CMD4 %.2X", cmd4_display_on[0]);
	et6220_command(dev->spi_fd, cmd4_display_on, rd_buff, 1);

}

/* Low level communication support */
uint8_t et6220_command(spi_et6220_device *dev, uint8_t tx[], uint8_t *rx, uint8_t size)
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
		.delay_usecs = dev->spi_delay,
		.speed_hz = dev->spi_speed_hz,
		.bits_per_word = 8
	};
	ret = ioctl(dev->spi_fd, SPI_IOC_MESSAGE(1), &tr);
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