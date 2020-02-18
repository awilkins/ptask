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
#define _XOPEN_SOURCE
#include <time.h>

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <log.h>
#include <ptime.h>
#include <settings.h>
#include <ui.h>
#include <ui_projecttree.h>
#include <ui_taskpanel.h>
#include <ui_tasktree.h>

static const char * const MENU_NAMES[] = {
	"menu_id_visible",
	"menu_description_visible",
	"menu_project_visible",
	"menu_uuid_visible",
	"menu_priority_visible",
	"menu_urgency_visible",
	"menu_creation_date_visible",
	"menu_due_visible",
	"menu_start_visible",
};

static GtkTreeView *w_treeview;
static GtkMenu *w_menu;
static struct task **current_tasks;
static gchar *search_keywords;

enum {
	COL_ID,
	COL_DESCRIPTION,
	COL_PROJECT,
	COL_UUID,
	COL_PRIORITY,
	COL_URGENCY,
	COL_CREATION_DATE,
	COL_DUE,
	COL_START,
	COL_COUNT
};

static GtkTreeViewColumn *w_cols[COL_COUNT];
static GtkCheckMenuItem *w_menus[COL_COUNT];

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

static gint priority_cmp(GtkTreeModel *model,
			 GtkTreeIter *a,
			 GtkTreeIter *b,
			 gpointer user_data)
{
	GValue v1 = {0,}, v2 = {0,};
	const char *str1, *str2;
	int i1, i2;

	gtk_tree_model_get_value(model, a, COL_PRIORITY, &v1);
	str1 = g_value_get_string(&v1);
	i1 = priority_to_int(str1);

	gtk_tree_model_get_value(model, b, COL_PRIORITY, &v2);
	str2 = g_value_get_string(&v2);
	i2 = priority_to_int(str2);

	if (i1 < i2)
		return -1;
	else if (i1 > i2)
		return 1;
	else
		return 0;
}

int tasktree_cursor_changed_cbk(GtkTreeView *treeview, gpointer data)
{
	log_fct_enter();

	ui_taskpanel_update(ui_tasktree_get_selected_task());

	log_fct_exit();

	return FALSE;
}

void ui_tasktree_init(GtkBuilder *builder)
{
	GtkTreeModel *model;
	int i;

	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tasktree"));
	w_menu = GTK_MENU(gtk_builder_get_object(builder, "tasktree_menu"));
	g_object_ref(w_menu);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model),
					COL_PRIORITY,
					priority_cmp,
					NULL,
					NULL);

	w_cols[COL_ID] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_id"));
	w_cols[COL_DESCRIPTION] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_description"));
	w_cols[COL_PROJECT] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_project"));
	w_cols[COL_UUID] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_uuid"));
	w_cols[COL_PRIORITY] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_priority"));
	w_cols[COL_URGENCY] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_urgency"));
	w_cols[COL_CREATION_DATE] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_creation_date"));
	w_cols[COL_DUE] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_due"));
	w_cols[COL_START] = GTK_TREE_VIEW_COLUMN
		(gtk_builder_get_object(builder, "col_start"));

	for (i = 0; i < COL_COUNT; i++)
		w_menus[i] = GTK_CHECK_MENU_ITEM
			(gtk_builder_get_object(builder, MENU_NAMES[i]));
}

void ui_tasktree_load_settings()
{
	int sort_col_id, i;
	GtkSortType sort_order;
	GtkTreeModel *model;
	const char *key;
	gboolean b;

	sort_col_id = settings_get_int(SETTINGS_KEY_TASKS_SORT_COL);
	sort_order = settings_get_int(SETTINGS_KEY_TASKS_SORT_ORDER);
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
					     sort_col_id, sort_order);


	for (i = 0; i < COL_COUNT; i++) {
		key = SETTINGS_VISIBLE_COL_KEYS[i];
		b = settings_get_boolean(key);
		gtk_tree_view_column_set_visible(w_cols[i], b);
	}

	for (i = 0; i < COL_COUNT; i++) {
		key = SETTINGS_VISIBLE_COL_KEYS[i];
		b = settings_get_boolean(key);
		gtk_check_menu_item_set_active(w_menus[i], b);
	}
}

void ui_tasktree_save_settings()
{
	int sort_col_id;
	GtkTreeModel *model;
	GtkSortType sort_order;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(model),
					     &sort_col_id,
					     &sort_order);
	log_debug("ui_tasktree_save_settings(): sort_col_id=%d", sort_col_id);
	log_debug("ui_tasktree_save_settings(): sort_col_order=%d", sort_order);

	settings_set_int(SETTINGS_KEY_TASKS_SORT_COL, sort_col_id);
	settings_set_int(SETTINGS_KEY_TASKS_SORT_ORDER, sort_order);
}

const char *ui_tasktree_get_task_uuid()
{
	struct task *t;

	t = ui_tasktree_get_selected_task();

	if (t)
		return t->uuid;
	else
		return NULL;
}

struct task *ui_tasktree_get_selected_task()
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	struct task **tasks_cur, *result;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *uuid;

	log_fct_enter();

	result = NULL;

	if (current_tasks) {
		gtk_tree_view_get_cursor(w_treeview, &path, &cols);

		if (path) {
			model = gtk_tree_view_get_model(w_treeview);
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get_value(model,
						 &iter,
						 COL_UUID,
						 &value);

			uuid = g_value_get_string(&value);

			for (tasks_cur = current_tasks; *tasks_cur; tasks_cur++)
				if (!strcmp((*tasks_cur)->uuid, uuid))
					result = *tasks_cur;

			gtk_tree_path_free(path);
		}
	}

	log_fct_exit();

	return result;
}

void ui_tasktree_set_selected_task(const char *uuid)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *c_uuid;

	log_fct_enter();

	if (current_tasks) {
		model = gtk_tree_view_get_model(w_treeview);

		if (!gtk_tree_model_get_iter_first(model, &iter))
			return ;

		path = NULL;
		while (gtk_tree_model_iter_next(model, &iter)) {
			gtk_tree_model_get_value(model,
						 &iter,
						 COL_UUID,
						 &value);
			c_uuid = g_value_get_string(&value);

			if (!strcmp(uuid, c_uuid)) {
				path = gtk_tree_model_get_path(model, &iter);
				break;
			}

			g_value_unset(&value);
		}

		if (!path)
			path = gtk_tree_path_new_first();
		gtk_tree_view_set_cursor(w_treeview, path, NULL, FALSE);
	}

	log_fct_exit();
}

static int match_search_keywords(struct task *task)
{
	gchar *desc;
	int ret;
	char **tags;
	gchar *tag;

	if (!search_keywords || !strlen(search_keywords))
		return 1;

	if (!task->description || !strlen(task->description))
		return 0;

	desc = g_ascii_strup(task->description, -1);

	if (strstr(desc, search_keywords))
		ret = 1;
	else
		ret = 0;

	free(desc);

	if (ret)
		return 1;

	tags = task->tags;
	if (!tags)
		return 0;

	while (*tags) {
		tag = g_ascii_strup(*tags, -1);

		if (strstr(tag, search_keywords))
			ret = 1;

		free(tag);

		if (ret)
			return 1;

		tags++;
	}

	return 0;
}

void ui_tasktree_update(struct task **tasks)
{
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;
	GtkTreeIter iter;
	const char *prj, *prj_filter;
	char *s;

	prj_filter = ui_projecttree_get_project();

	current_tasks = tasks;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	if (current_tasks) {
		for (tasks_cur = current_tasks; *tasks_cur; tasks_cur++) {
			task = (*tasks_cur);

			if (task->project)
				prj = task->project;
			else
				prj = "";

			if (prj_filter && strcmp(prj, prj_filter))
				continue;

			if (!match_search_keywords(task))
				continue;

			gtk_list_store_append(GTK_LIST_STORE(model), &iter);

			gtk_list_store_set(GTK_LIST_STORE(model),
					   &iter,
					   COL_ID,
					   (*tasks_cur)->id,
					   COL_DESCRIPTION,
					   (*tasks_cur)->description,
					   COL_PROJECT,
					   prj,
					   COL_UUID,
					   (*tasks_cur)->uuid,
					   COL_PRIORITY,
					   (*tasks_cur)->priority,
					   COL_URGENCY,
					   (*tasks_cur)->urgency,
					   -1);

			if ((*tasks_cur)->start) {
				s = tm_to_str((*tasks_cur)->start);
				gtk_list_store_set
					(GTK_LIST_STORE(model),
					 &iter,
					 COL_START,
					 s,
					 -1);
				free(s);
			}

			if ((*tasks_cur)->due) {
				s = tm_to_str((*tasks_cur)->due);
				gtk_list_store_set
					(GTK_LIST_STORE(model),
					 &iter,
					 COL_DUE,
					 s,
					 -1);
				free(s);
			}

			if ((*tasks_cur)->entry) {
				s = tm_to_str((*tasks_cur)->entry);
				gtk_list_store_set
					(GTK_LIST_STORE(model),
					 &iter,
					 COL_CREATION_DATE,
					 s,
					 -1);
				free(s);
			}
		}
	}

}

gboolean tasktree_button_press_event_cbk(GtkWidget *widget,
					 GdkEventButton *evt,
					 gpointer data)
{
	log_fct_enter();

	if (evt->button == 3)
		gtk_menu_popup(w_menu,
			       NULL, NULL, NULL, NULL, evt->button, evt->time);

	log_fct_exit();

	return FALSE;
}

void tasktree_visible_activate_cbk(GtkAction *action, gpointer data)
{
	gboolean b;
	int id;
	const char *aname, *key;

	aname = gtk_action_get_name(action);

	if (!strcmp(aname, "tasktree_id_visible"))
		id = COL_ID;
	else if (!strcmp(aname, "tasktree_description_visible"))
		id = COL_DESCRIPTION;
	else if (!strcmp(aname, "tasktree_project_visible"))
		id = COL_PROJECT;
	else if (!strcmp(aname, "tasktree_uuid_visible"))
		id = COL_UUID;
	else if (!strcmp(aname, "tasktree_priority_visible"))
		id = COL_PRIORITY;
	else if (!strcmp(aname, "tasktree_urgency_visible"))
		id = COL_URGENCY;
	else if (!strcmp(aname, "tasktree_creation_date_visible"))
		id = COL_CREATION_DATE;
	else if (!strcmp(aname, "tasktree_due_visible"))
		id = COL_DUE;
	else if (!strcmp(aname, "tasktree_start_visible"))
		id = COL_START;
	else
		id = -1;

	if (id != -1) {
		key = SETTINGS_VISIBLE_COL_KEYS[id];
		b = settings_get_boolean(key);
		settings_set_boolean(key, !b);
		gtk_tree_view_column_set_visible(w_cols[id], !b);
	}
}

void tasktree_done_activate_cbk(GtkAction *action, gpointer data)
{
	struct task *t;

	log_fct_enter();

	t = ui_tasktree_get_selected_task();

	if (t) {
		tw_task_done(t->uuid);
		refresh();
	}

	log_fct_exit();
}

void tasktree_start_activate_cbk(GtkAction *action, gpointer data)
{
	struct task *t;

	log_fct_enter();

	t = ui_tasktree_get_selected_task();

	if (t) {
		tw_task_start(t->uuid);
		refresh();
	}

	log_fct_exit();
}

void tasktree_stop_activate_cbk(GtkAction *action, gpointer data)
{
	struct task *t;

	log_fct_enter();

	t = ui_tasktree_get_selected_task();

	if (t) {
		tw_task_stop(t->uuid);
		refresh();
	}

	log_fct_exit();
}

void
ui_tasktree_search_changed_cbk(GtkEntry *entry, gchar *preedit, gpointer data)
{
	if (search_keywords)
		g_free(search_keywords);

	search_keywords = g_ascii_strup(gtk_entry_get_text(entry), -1);

	ui_tasktree_update(current_tasks);
}

void ui_tasktree_update_filter(const char *prj)
{
	ui_tasktree_update(current_tasks);
}
