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
#include <config.h>

#include <stdlib.h>

#include <glib/gi18n.h>

#include <log.h>
#include <settings.h>
#include <ui.h>
#include <ui_newtask_diag.h>
#include <ui_projecttree.h>
#include <ui_taskpanel.h>
#include <ui_tasktree.h>

static GtkComboBox *w_status;
static GtkWindow *window;
static GtkPaned *vpaned;
static GtkPaned *hpaned;

int newtask_clicked_cbk(GtkButton *btn, gpointer data)
{
	ui_newtask();

	return FALSE;
}

static void save_settings(GtkWindow *window)
{
	int w, h, x, y, pos;

	gtk_window_get_size(window, &w, &h);
	gtk_window_get_position(window, &x, &y);

	log_fct("x=%d, y=%d, w=%d, h=%d", x, y, w, h);

	settings_set_int(SETTINGS_KEY_WINDOW_WIDTH, w);
	settings_set_int(SETTINGS_KEY_WINDOW_HEIGHT, h);
	settings_set_int(SETTINGS_KEY_WINDOW_X, x);
	settings_set_int(SETTINGS_KEY_WINDOW_Y, y);

	pos = gtk_paned_get_position(vpaned);
	settings_set_int(SETTINGS_KEY_SPLITER_VERTICAL_POS, pos);

	pos = gtk_paned_get_position(hpaned);
	settings_set_int(SETTINGS_KEY_SPLITER_HORIZONTAL_POS, pos);

	ui_tasktree_save_settings();

	g_settings_sync();
}

int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
	log_fct_enter();
	refresh();
	log_fct_exit();
	return FALSE;
}


static void ui_quit()
{
	save_settings(window);
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_main_quit();
}

static gboolean delete_event_cbk(GtkWidget *w, GdkEvent *evt, gpointer data)
{
	log_fct_enter();

	ui_quit();

	log_fct_exit();

	return TRUE;
}

static int status_changed_cbk(GtkComboBox *w, gpointer data)
{
	log_fct_enter();

	refresh();

	log_fct_exit();

	return FALSE;
}

GtkWindow *create_window(GtkBuilder *builder)
{
	int x, y, w, h, pos;

	window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));

	w_status = GTK_COMBO_BOX(gtk_builder_get_object(builder, "status"));
	g_signal_connect(w_status,
			 "changed", (GCallback)status_changed_cbk,
			 NULL);

	w = settings_get_int(SETTINGS_KEY_WINDOW_WIDTH);
	h = settings_get_int(SETTINGS_KEY_WINDOW_HEIGHT);
	gtk_window_set_default_size(window, w, h);

	x = settings_get_int(SETTINGS_KEY_WINDOW_X);
	y = settings_get_int(SETTINGS_KEY_WINDOW_Y);
	gtk_window_move(window, x, y);

	vpaned = GTK_PANED(gtk_builder_get_object(builder, "vpaned"));
	pos = settings_get_int(SETTINGS_KEY_SPLITER_VERTICAL_POS);
	gtk_paned_set_position(vpaned, pos);

	hpaned = GTK_PANED(gtk_builder_get_object(builder, "hpaned"));
	pos = settings_get_int(SETTINGS_KEY_SPLITER_HORIZONTAL_POS);
	gtk_paned_set_position(hpaned, pos);

	g_signal_connect(window, "delete_event",
			 G_CALLBACK(delete_event_cbk), NULL);

	ui_taskpanel_init(builder);
	ui_tasktree_init(builder);
	ui_projecttree_init(builder);

	ui_tasktree_load_settings();

	return window;
}

const char *ui_get_status_filter()
{
	const char *status;

	log_fct_enter();

	status = gtk_combo_box_get_active_id(w_status);
	log_fct("status: %s", status);

	log_fct_exit();

	return status;
}

void quit_activate_cbk(GtkWidget *menu_item, gpointer data)
{
	log_fct_enter();
	ui_quit();
	log_fct_exit();
}

void preferences_activate_cbk(GtkWidget *menu_item, gpointer data)
{
	gint result;
	static GtkDialog *diag;
	GtkBuilder *builder;
	GtkFileChooser *w_dir;
	char *dir;
	const char *sdir;

	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 NULL);
	diag = GTK_DIALOG(gtk_builder_get_object(builder, "diag_preferences"));
	gtk_builder_connect_signals(builder, NULL);

	w_dir = GTK_FILE_CHOOSER(gtk_builder_get_object(builder,
							"dir_chooser"));

	sdir = settings_get_notes_dir();
	if (sdir && *sdir)
		gtk_file_chooser_set_filename(w_dir, sdir);

	result = gtk_dialog_run(diag);

	if (result) {
		log_debug("preferences_activate_cbk(): accept");
		dir = gtk_file_chooser_get_filename(w_dir);

		if (dir) {
			log_debug("preferences_activate_cbk(): path=%s", dir);
			settings_set_notes_dir(dir);
			free(dir);
		}

		refresh();
	} else {
		log_debug("preferences_activate_cbk(): cancel");
	}

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));
}

void about_activate_cbk(GtkWidget *menu_item, gpointer data)
{
	log_fct_enter();

	gtk_show_about_dialog
		(NULL,
		 "comments",
		 _("ptask is a GTK+ task management application"),
		 "copyright",
		 _("Copyright(c) 2010-2016\njeanfi@gmail.com"),
		 "logo-icon-name", "ptask",
		 "program-name", "ptask",
		 "title", _("About ptask"),
		 "version", VERSION,
		 "website", PACKAGE_URL,
		 "website-label", _("ptask Homepage"),
		 NULL);

	log_fct_exit();
}
