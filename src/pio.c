/*
 * Copyright (C) 2011-2014 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <pio.h>

/* Directory separator is \ when cross-compiling for MS Windows
   systems */
#if defined(__MINGW32__)
#define DIRSEP ('\\')
#else
#define DIRSEP '/'
#endif

void mkdirs(const char *dirs, mode_t mode)
{
	char *c = (char *)dirs;
	char *dir = malloc(strlen(dirs) + 1);

	int i = 0;
	while (*c) {
		if ((*c == DIRSEP || *c == '\0') && c != dirs) {
			strncpy(dir, dirs, i);
			dir[i] = '\0';
			mkdir(dir, mode);
		}

		c++;
		i++;
	}

	mkdir(dirs, mode);

	free(dir);
}
