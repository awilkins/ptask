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

#include <string.h>

#include <gtk/gtk.h>

#include <log.h>
#include <tw.h>
#include <ui.h>

static const char *ui_get_priority(GtkComboBox *combo)
{
	int prio;

	prio = gtk_combo_box_get_active(combo);

	switch (prio) {
	case 3:
		return "H";
	case 2:
		return "M";
	case 1:
		return "L";
	default:
		return "";
	}
}

static void populate_project(GtkComboBoxText *w)
{
	struct task **tasks;
	struct project **all_prjs, **prjs;

	tasks = tw_get_all_tasks("pending");

	all_prjs = tw_get_projects(tasks);

	prjs = all_prjs;
	while (*prjs) {
		if (strcmp((*prjs)->name, "ALL"))
			gtk_combo_box_text_append_text(w, (*prjs)->name);

		prjs++;
	}

	tw_task_list_free(tasks);
	tw_project_list_free(all_prjs);
}

void ui_newtask()
{
	gint result;
	static GtkDialog *diag;
	GtkBuilder *builder;
	GtkEntry *entry;
	const char *desc, *prj, *prio;
	GtkComboBox *combo;
	GtkComboBoxText *w_prj;

	log_debug("ui_newtask()");

	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 NULL);
	diag = GTK_DIALOG(gtk_builder_get_object(builder, "diag_tasknew"));
	gtk_builder_connect_signals(builder, NULL);

	w_prj = GTK_COMBO_BOX_TEXT(gtk_builder_get_object
				   (builder, "diag_tasknew_project"));
	populate_project(w_prj);

	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {
		log_debug("ui_newtask(): ok");

		entry = GTK_ENTRY(gtk_builder_get_object
				  (builder, "diag_tasknew_description"));
		desc = gtk_entry_get_text(entry);

		w_prj = GTK_COMBO_BOX_TEXT(gtk_builder_get_object
					   (builder, "diag_tasknew_project"));
		prj = gtk_combo_box_text_get_active_text(w_prj);

		combo = GTK_COMBO_BOX(gtk_builder_get_object
				      (builder, "diag_tasknew_priority"));
		prio = ui_get_priority(combo);

		log_debug("ui_newtask(): description=%s project=%s priority=%d",
			  desc,
			  prj,
			  prio);

		tw_add(desc, prj, prio);
		refresh();
	} else {
		log_debug("ui_newtask(): cancel");
	}

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));
}
