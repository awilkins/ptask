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

#include <settings.h>

const char *SETTINGS_KEY_WINDOW_WIDTH = "window-width";
const char *SETTINGS_KEY_WINDOW_HEIGHT = "window-height";
const char *SETTINGS_KEY_WINDOW_X = "window-x";
const char *SETTINGS_KEY_WINDOW_Y = "window-y";
const char *SETTINGS_KEY_SPLITER_VERTICAL_POS = "spliter-vertical-pos";
const char *SETTINGS_KEY_SPLITER_HORIZONTAL_POS = "spliter-horizontal-pos";
const char *SETTINGS_KEY_TASKS_SORT_COL = "tasks-sort-col";
const char *SETTINGS_KEY_TASKS_SORT_ORDER = "tasks-sort-order";
const char *SETTINGS_KEY_NOTES_DIR = "notes-dir";

const char * const SETTINGS_VISIBLE_COL_KEYS[] = {
	"tasktree-id-visible",
	"tasktree-description-visible",
	"tasktree-project-visible",
	"tasktree-uuid-visible",
	"tasktree-priority-visible",
	"tasktree-urgency-visible",
	"tasktree-creation-date-visible",
	"tasktree-due-visible",
	"tasktree-start-visible",
};

static GSettings *settings;

void settings_init()
{
	settings = g_settings_new("ptask");
}

gint settings_get_int(const gchar *key)
{
	return g_settings_get_int(settings, key);
}

void settings_set_int(const gchar *key, gint value)
{
	g_settings_set_int(settings, key, value);
}

gboolean settings_get_boolean(const gchar *key)
{
	return g_settings_get_boolean(settings, key);
}

void settings_set_boolean(const gchar *key, gboolean value)
{
	g_settings_set_boolean(settings, key, value);
}

gchar *settings_get_str(const gchar *key)
{
	return g_settings_get_string(settings, key);
}

void settings_set_str(const gchar *key, const gchar *value)
{
	g_settings_set_string(settings, key, value);
}

const char *settings_get_notes_dir()
{
	return settings_get_str(SETTINGS_KEY_NOTES_DIR);
}

void settings_set_notes_dir(const char *dir)
{
	settings_set_str(SETTINGS_KEY_NOTES_DIR, dir);
}
