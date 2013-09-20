#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/types.h>
#include "lib/et6220.h"
#include "lib/segment_parser.h"

int main(int argc, char *argv[])
{
	char cadena[] = "0:00:0#abcdde#";
	uint8_t char_pos = 0;
	uint8_t segment_pos = 1;
	et6220_display_data data = {
		.g1 = 0,
		.g2 = 0,
		.g3 = 0,
		.g4 = 0,
		.g5 = 0
	};
	parse(cadena, &char_pos, &segment_pos, &data);
	
	return 0;
}