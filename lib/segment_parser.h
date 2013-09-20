/*
 * et6220 parser library headers
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
#include <linux/types.h>
#include "et6220.h"

void parse(char *str, uint8_t *char_pos, uint8_t *segment_pos, et6220_display_data *data);
