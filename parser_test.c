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
	char cadena[] = "0:00:0#abcdef.#";
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
	
	uint16_t zero = (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_D|SEGMENT_E|SEGMENT_F);
	et6220_display_data expected_data = {
		.g1 = zero,
		.g2 = zero,
		.g3 = zero|SEGMENT_DOT,
		.g4 = zero,
		.g5 = zero|SEGMENT_DOT
	};
	printf("G1 got: 0x%4X expected: 0x%4X \n", data.g1, expected_data.g1);
	printf("G2 got: 0x%4X expected: 0x%4X \n", data.g2, expected_data.g2);
	printf("G3 got: 0x%4X expected: 0x%4X \n", data.g3, expected_data.g3);
	printf("G4 got: 0x%4X expected: 0x%4X \n", data.g4, expected_data.g4);
	printf("G5 got: 0x%4X expected: 0x%4X \n", data.g5, expected_data.g5);
	return 0;
}
