/*
 * Copyright (C) 2012-2016 jeanfi@gmail.com
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

#ifndef _PTASK_SETTINGS_H_
#define _PTASK_SETTINGS_H_

#include <gtk/gtk.h>

extern const char *SETTINGS_KEY_WINDOW_WIDTH;
extern const char *SETTINGS_KEY_WINDOW_HEIGHT;
extern const char *SETTINGS_KEY_WINDOW_X;
extern const char *SETTINGS_KEY_WINDOW_Y;
extern const char *SETTINGS_KEY_SPLITER_VERTICAL_POS;
extern const char *SETTINGS_KEY_SPLITER_HORIZONTAL_POS;
extern const char *SETTINGS_KEY_TASKS_SORT_COL;
extern const char *SETTINGS_KEY_TASKS_SORT_ORDER;
extern const char * const SETTINGS_VISIBLE_COL_KEYS[];

void settings_init();

gint settings_get_int(const gchar *key);
void settings_set_int(const gchar *key, gint value);
gboolean settings_get_boolean(const gchar *key);
void settings_set_boolean(const gchar *key, gboolean value);
gchar *settings_get_str(const gchar *key);
void settings_set_str(const gchar *key, const gchar *value);

const char *settings_get_notes_dir();
void settings_set_notes_dir(const char *dir);

#endif
