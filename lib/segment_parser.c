/*
 * et6220 parser library
 * 
 * Copyright (c) 2013 Jesus Trujillo <elyeyus@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include "segment_parser.h"

/* 
 * Receives a character as a parameter an returns a uint16_t
 * mapping according to et6220 display
 */
uint16_t get_character(u_char character) 
{
	uint16_t ret = 0x00;
	switch(character) {
		case '0':
			ret = (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_D|SEGMENT_E|SEGMENT_F);
			break;
		case '1':
			ret = (SEGMENT_B|SEGMENT_C);
			break;
		case '2':
			ret = (SEGMENT_A|SEGMENT_B|SEGMENT_D|SEGMENT_E|SEGMENT_G); 
			break;
		case '3':
			ret = (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_D|SEGMENT_G);
			break;
		case '4':
			ret = (SEGMENT_B|SEGMENT_C|SEGMENT_F|SEGMENT_G);
			break;
		case '5':
			ret = (SEGMENT_A|SEGMENT_F|SEGMENT_C|SEGMENT_D|SEGMENT_G);
			break;
		case '6':
			ret = (SEGMENT_A|SEGMENT_C|SEGMENT_D|SEGMENT_E|SEGMENT_F|SEGMENT_G);
			break;
		case '7':
			ret = (SEGMENT_A|SEGMENT_B|SEGMENT_C);
			break;
		case '8':
			ret = (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_D|SEGMENT_E|SEGMENT_F|SEGMENT_G);
			break;
		case '9':
			ret = (SEGMENT_A|SEGMENT_B|SEGMENT_C|SEGMENT_F|SEGMENT_G);
			break;
		default:
			ret = 0;
	}
	return ret;
}

/*
 * Sets the colon segment in the selected position
 */ 
void set_colon(uint8_t segment_pos, et6220_display_data *data)
{
	if(segment_pos == 3) {
		data->g3 = data->g3|SEGMENT_DOT;
	} else if (segment_pos == 5) {
		data->g5 = data->g5|SEGMENT_DOT;
	}
}

/*
 * Sets the segment information into the selected position
 */
void set_segment(uint8_t segment_pos, et6220_display_data *data, uint16_t segment_data)
{
	switch(segment_pos) {
		case 1:
			data->g1 = segment_data;
			break;
		case 2:
			data->g2 = segment_data;
			break;
		case 3:
			data->g3 = segment_data;
			break;
		case 4:
			data->g4 = segment_data;
			break;
		case 5:
			data->g5 = segment_data;
			break;
	}	
}

/*
 * Parse custom segment 
 */
void parse_custom_segment(u_char *str, uint8_t *char_pos, uint8_t *segment_pos, et6220_display_data *data) 
{
	uint8_t iterate = 1;
	uint16_t segment_data = 0;

	while(iterate) {
		uint8_t chp = str[*char_pos];
		int ch = chp;
		switch(ch) {
			case 'a':
				segment_data |= SEGMENT_A;
				break;
			case 'b':
				segment_data |= SEGMENT_B;
				break;
			case 'c':
				segment_data |= SEGMENT_C;
				break;
			case 'd':
				segment_data |= SEGMENT_D;
				break;
			case 'e':
				segment_data |= SEGMENT_E;
				break;
			case 'f':
				segment_data |= SEGMENT_F;
				break;
			case 'g':
				segment_data |= SEGMENT_F;
				break;
			case '#':
				// end of definition write to segment and end
				iterate = 0;
				set_segment(*segment_pos, data, segment_data);
				break;
			default:
				return;
		}
		*char_pos++;
	}

}

void parse(u_char *str, uint8_t *char_pos, uint8_t *segment_pos, et6220_display_data *data) 
{

	u_char ch = str[*char_pos];
	
	// consume character
	if(get_character(ch) != 0) {
		// our character is in the lookup so we set it
		set_segment(*segment_pos, data, get_character(ch));
	} else if (ch == ':') {
		// our character is the colon		
		set_colon(segment_pos, data);
	} else if (ch == '#') {
		// we detected a segment definition start
		char_pos++;
		ch = str[*char_pos];
		// start parsing custom segment data
		parse_custom_segment(str, char_pos, segment_pos, data);
	} else if (ch == '\0') {
		// exit when NULL character found
		return;
	} else {
		// exit when non recognizable character found
		return;
	}

	// increment char_pos and segment
	*char_pos++;
	*segment_pos++;
	parse(str, char_pos, segment_pos, data);

}