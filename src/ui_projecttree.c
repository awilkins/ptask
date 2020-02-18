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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>
#include <ui_projecttree.h>
#include <ui_tasktree.h>

enum {
	COL_NAME,
	COL_COUNT
};

static GtkTreeView *w_treeview;

static int cursor_changed_cbk(GtkTreeView *treeview, gpointer data)
{
	const char *prj;

	log_fct_enter();

	prj = ui_projecttree_get_project();

	ui_tasktree_update_filter(prj);

	log_fct_exit();

	return FALSE;
}

void ui_projecttree_init(GtkBuilder *builder)
{
	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder,
							  "projecttree"));
	g_signal_connect(w_treeview,
			 "cursor-changed", (GCallback)cursor_changed_cbk,
			 NULL);
}

const char *ui_projecttree_get_project()
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *prj;

	log_fct_enter();

	gtk_tree_view_get_cursor(w_treeview, &path, &cols);

	if (path) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get_value(model, &iter, COL_NAME, &value);

		prj = g_value_get_string(&value);

		if (!strcmp(prj, "ALL"))
			prj = NULL;
	} else {
		prj = NULL;
	}

	log_fct_exit();

	return prj;
}


void ui_projecttree_update(struct task **ts)
{
	struct project **prjs, **cur;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *p;
	const char *current_prj;

	log_fct_enter();

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));

	current_prj = ui_projecttree_get_project();

	gtk_list_store_clear(GTK_LIST_STORE(model));

	prjs = tw_get_projects(ts);
	for (cur = prjs; *cur; cur++) {
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter,
				   COL_NAME, (*cur)->name,
				   COL_COUNT, (*cur)->count,
				   -1);

		if (current_prj) {
			if (!strcmp((*cur)->name, current_prj)) {
				p = gtk_tree_model_get_path(model, &iter);
				if (p) {
					gtk_tree_view_set_cursor(w_treeview,
								 p,
								 NULL,
								 FALSE);
				}
			}
		}
	}

	tw_project_list_free(prjs);

	log_fct_exit();
}

