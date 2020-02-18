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

#include <log.h>
#include <note.h>
#include <tw.h>
#include <ui.h>
#include <ui_taskpanel.h>

static GtkTextView *w_note;
static GtkEntry *w_description;
static GtkEntry *w_project;
static GtkComboBox *w_priority;
static GtkButton *w_tasksave_btn;
static GtkButton *w_taskremove_btn;
static GtkButton *w_taskdone_btn;
static GtkButton *w_taskcancel_btn;
static GtkLabel *w_tasktags;

static struct task *current_task;

static void enable(int enable)
{
	GtkTextBuffer *buf;

	gtk_widget_set_sensitive(GTK_WIDGET(w_tasksave_btn), enable);
	gtk_widget_set_sensitive(GTK_WIDGET(w_taskdone_btn), enable);

	if (current_task && current_task->recur) {
		gtk_widget_set_sensitive(GTK_WIDGET(w_taskremove_btn), FALSE);
		gtk_widget_set_tooltip_text
			(GTK_WIDGET(w_taskremove_btn),
			 "The removal of recurrent tasks is not supported due "
			 "to the taskwarrior bug TW-638");
		gtk_widget_set_has_tooltip(GTK_WIDGET(w_taskremove_btn), TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(w_taskremove_btn), enable);
		gtk_widget_set_has_tooltip(GTK_WIDGET(w_taskremove_btn), FALSE);
	}

	gtk_widget_set_sensitive(GTK_WIDGET(w_taskcancel_btn), enable);

	buf = gtk_text_view_get_buffer(w_note);
	if (!enable)
		gtk_text_buffer_set_text(buf, "", 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_note), enable);

	if (!enable)
		gtk_entry_set_text(w_description, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_description), enable);

	if (!enable)
		gtk_entry_set_text(w_project, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_project), enable);

	if (!enable)
		gtk_label_set_label(w_tasktags, "");

	if (!enable)
		gtk_combo_box_set_active(w_priority, 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_priority), enable);
}

static int tasksave_clicked_cbk(GtkButton *btn, gpointer data)
{
	struct task *task;
	GtkTextBuffer *buf;
	char *txt, *pri;
	GtkTextIter sIter, eIter;
	const char *ctxt;
	int priority;

	log_fct_enter();

	task = current_task;

	log_fct("%d", task->id);

	buf = gtk_text_view_get_buffer(w_note);

	gtk_text_buffer_get_iter_at_offset(buf, &sIter, 0);
	gtk_text_buffer_get_iter_at_offset(buf, &eIter, -1);
	txt = gtk_text_buffer_get_text(buf, &sIter, &eIter, TRUE);

	log_debug("note=%s", txt);

	if (!task->note || strcmp(txt, task->note))
		note_put(task->uuid, txt);

	ctxt = gtk_entry_get_text(w_description);
	if (!task->description || strcmp(ctxt, task->description))
		tw_modify_description(task->uuid, ctxt);

	ctxt = gtk_entry_get_text(w_project);
	if (!task->project || strcmp(ctxt, task->project))
		tw_modify_project(task->uuid, ctxt);

	priority = gtk_combo_box_get_active(w_priority);
	log_debug("priority: %d", priority);

	switch (priority) {
	case 3:
		pri = "H";
		break;
	case 2:
		pri = "M";
		break;
	case 1:
		pri = "L";
		break;
	default:
		pri = "";
	}

	if (strcmp(task->priority, pri))
		tw_modify_priority(task->uuid, pri);

	refresh();

	return FALSE;
}

void ui_taskpanel_init(GtkBuilder *builder)
{
	log_fct("ENTER");

	w_note = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "tasknote"));
	w_tasktags = GTK_LABEL(gtk_builder_get_object(builder, "tasktags"));
	w_description = GTK_ENTRY(gtk_builder_get_object(builder,
							 "taskdescription"));
	w_project = GTK_ENTRY(gtk_builder_get_object(builder, "taskproject"));
	w_priority = GTK_COMBO_BOX(gtk_builder_get_object(builder,
							  "taskpriority"));

	w_tasksave_btn = GTK_BUTTON(gtk_builder_get_object(builder,
							   "tasksave"));
	w_taskremove_btn = GTK_BUTTON(gtk_builder_get_object(builder,
							     "taskremove"));

	g_signal_connect(w_tasksave_btn,
			 "clicked",
			 (GCallback)tasksave_clicked_cbk,
			 NULL);

	w_taskdone_btn = GTK_BUTTON(gtk_builder_get_object(builder,
							   "taskdone"));
	w_taskcancel_btn = GTK_BUTTON(gtk_builder_get_object(builder,
							     "taskcancel"));

	enable(0);

	log_fct("EXIT");
}

static int priority_to_int(const char *str)
{
	switch (*str) {
	case 'H':
		return 3;
	case 'M':
		return 2;
	case 'L':
		return 1;
	default:
		return 0;
	}
}

void ui_taskpanel_update(struct task *task)
{
	GtkTextBuffer *buf;
	int priority;
	char **tags;
	gchar *tmp, *gtags;

	if (task) {
		current_task = task;

		buf = gtk_text_view_get_buffer(w_note);
		if (task->note)
			gtk_text_buffer_set_text(buf,
						 task->note,
						 strlen(task->note));
		else
			gtk_text_buffer_set_text(buf, "", 0);

		gtk_entry_set_text(w_description, task->description);

		if (task->project)
			gtk_entry_set_text(w_project, task->project);
		else
			gtk_entry_set_text(w_project, "");

		priority = priority_to_int(task->priority);
		gtk_combo_box_set_active(w_priority, priority);

		tags = task->tags;
		gtags = NULL;
		if (tags) {
			while (*tags) {
				if (gtags) {
					tmp = g_strconcat(gtags,
							  " ",
							  *tags,
							  NULL);
					g_free(gtags);
					gtags = tmp;
				} else {
					gtags = g_strdup(*tags);
				}
				tags++;
			}
			gtk_label_set_label(w_tasktags, gtags);
		} else {
			gtk_label_set_label(w_tasktags, "");
		}

		enable(1);
	} else {
		current_task = NULL;
		enable(0);
	}
}

int taskdone_clicked_cbk(GtkButton *btn, gpointer data)
{
	if (current_task) {
		tw_task_done(current_task->uuid);
		refresh();
	}

	return FALSE;
}

int taskremove_clicked_cbk(GtkButton *btn, gpointer data)
{
	log_fct_enter();

	if (current_task) {
		log_fct("uuid=%d", current_task->uuid);
		tw_task_remove(current_task->uuid);
		refresh();
	}

	log_fct_exit();

	return FALSE;
}

int taskpanel_cancel_clicked_cbk(GtkButton *btn, gpointer data)
{
	log_fct_enter();

	ui_taskpanel_update(current_task);

	log_fct_exit();

	return FALSE;
}
