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
#include <locale.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#include <json.h>

#include <glib/gi18n.h>

#include <config.h>

#include <log.h>
#include <note.h>
#include <settings.h>
#include <tw.h>
#include <ui.h>
#include <ui_projecttree.h>
#include <ui_taskpanel.h>
#include <ui_tasktree.h>

static const char *program_name;
static struct task **tasks;

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"force-unsupported-taskwarrior", no_argument, 0, 'f'},
	{"debug", required_argument, 0, 'd'},
	{0, 0, 0, 0}
};

static void print_version()
{
	printf("ptask %s\n", VERSION);
	printf(_("Copyright (C) %s jeanfi@gmail.com\n"
		 "License GPLv2: GNU GPL version 2 or later "
		 "<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n"
		 "This is free software: you are free to change and "
		 " redistribute it.\n"
		 "There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2012-2016");
}

static void print_help()
{
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	puts(_("ptask is a task management user interface based"
	       " on taskwarrior."));

	puts("");
	puts(_("Options:"));
	puts(_("  -h, --help          display this help and exit\n"
	       "  -v, --version       display version information and exit"));

	puts("");
	puts(_("  -f, --force-unsupported-taskwarrior force usage of an"
	       " unsupported version of taskwarrior"));
	puts(_("  -d, --debug=LEVEL   "
	       "set the debug level, integer between 0 and 3"));

	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

void refresh()
{
	GtkWidget *dialog;
	const char *current_uuid;
	struct task **old_tasks;

	log_fct_enter();
	ui_taskpanel_update(NULL);

	if (tasks) {
		old_tasks = tasks;
		current_uuid = ui_tasktree_get_task_uuid();
		ui_tasktree_update(NULL);
	} else {
		old_tasks = NULL;
		current_uuid = NULL;
	}

	tasks = tw_get_all_tasks(ui_get_status_filter());

	if (tasks) {
		ui_projecttree_update(tasks);
		ui_tasktree_update(tasks);
		if (current_uuid)
			ui_tasktree_set_selected_task(current_uuid);
	} else {
		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("Error loading tasks, verify "
						  "that a supported version of "
						  "taskwarrior is installed."));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	if (old_tasks)
		tw_task_list_free(old_tasks);

	log_fct_exit();
}

static void log_init()
{
	char *home, *path, *dir;

	home = getenv("HOME");

	if (!home)
		return ;

	dir = malloc(strlen(home)+1+strlen(".ptask")+1);
	sprintf(dir, "%s/%s", home, ".ptask");
	mkdir(dir, 0777);

	path = malloc(strlen(dir)+1+strlen("log")+1);
	sprintf(path, "%s/%s", dir, "log");

	log_open(path);

	free(dir);
	free(path);
}

int main(int argc, char **argv)
{
	GtkWindow *window;
	GtkBuilder *builder;
	int optc, cmdok, opti, ret;
	GError *err;
	gchar *msg;
	GtkMessageDialog *diag;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	cmdok = 1;
	while ((optc = getopt_long(argc, argv, "vhfd:", long_options,
				   &opti)) != -1) {
		switch (optc) {
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'd':
			log_level = atoi(optarg);
			log_info(_("Enables debug mode."));
			break;
		case 'f':
			log_info(_("Force usage of an unsupported version of "
				   "taskwarrior."));
			tw_enable_check_version(0);
			break;
		default:
			cmdok = 0;
			break;
		}
	}

	if (!cmdok || optind != argc) {
		fprintf(stderr, _("Try `%s --help' for more information.\n"),
			program_name);
		exit(EXIT_FAILURE);
	}

	log_init();

	gtk_init(NULL, NULL);

	settings_init();

	builder = gtk_builder_new();
	err = NULL;
	ret = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 &err);

	if (!ret) {
		msg = g_strdup_printf(_("Failed to load UI: %s"),
				      err->message);
		log_err(msg);

		diag = GTK_MESSAGE_DIALOG(gtk_message_dialog_new
					  (NULL,
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_MESSAGE_ERROR,
					   GTK_BUTTONS_CLOSE,
					   msg,
					   NULL));

		gtk_dialog_run(GTK_DIALOG(diag));

		g_free(msg);

		exit(EXIT_FAILURE);
	}

	window = create_window(builder);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(G_OBJECT(builder));

	refresh();

	gtk_widget_show_all(GTK_WIDGET(window));

	gtk_main();

	exit(EXIT_SUCCESS);
}
