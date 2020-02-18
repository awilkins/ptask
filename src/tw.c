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
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <json.h>

#include <log.h>
#include "note.h"
#include <pstr.h>
#include "tw.h"

/* Whether ptask check that the taskwarrior version is supported. */
static int check_version_enabled = 1;

struct tm *parse_time(const char *t)
{
	struct tm *tm;

	tm = malloc(sizeof(struct tm));
	memset(tm, 0, sizeof(struct tm));
	strptime(t, "%Y%m%dT%H%M%S%Z", tm);

	return tm;
}

static char *task_exec(char *opts)
{
	FILE *f;
	int ret;
	size_t s;
	char *str, *tmp, *cmd, buf[1024];

	log_fct_enter();

	cmd = malloc(strlen("task ") + strlen(opts) + 1);
	strcpy(cmd, "task ");
	strcat(cmd, opts);

	log_debug("execute: %s", cmd);

	f = popen(cmd, "r");

	free(cmd);

	if (!f) {
		perror("popen");
		return NULL;
	}

	str = strdup("");
	while ((s = fread(buf, 1, 1024, f))) {
		tmp = malloc(strlen(str) + s + (size_t)1);
		memcpy(tmp, str, strlen(str));
		memcpy(tmp + strlen(str), buf, s);
		tmp[strlen(str) + s] = '\0';
		free(str);
		str = tmp;
	}

	ret = pclose(f);

	if (ret == -1)
		log_err("pclose fails");

	log_fct_exit();

	return str;
}

static char *task_get_version()
{
	char *out;

	out = task_exec("--version");

	trim(out);

	return out;
}

static int task_check_version()
{
	char *ver;

	ver = task_get_version();

	if (!ver)
		return 0;

	log_debug("task version: %s", ver);

	if (!strcmp(ver, "2.2.0")
	    || !strcmp(ver, "2.0.0")
	    || !strcmp(ver, "2.3.0")
	    || !strcmp(ver, "2.4.0")
	    || !strcmp(ver, "2.4.1")
	    || !strcmp(ver, "2.5.0"))
		return 1;
	else
		return 0;
}

static char *tw_exec(char *opts)
{
	char *opts2;

	if (check_version_enabled && !task_check_version()) {
		log_err("ptask is not compatible with the installed version of "
			"taskwarrior. The command line option -f can force "
			"the usage of an unsupported version of taskwarrior "
			"(risk of malfunction like damaging data).");
		return NULL;
	}

	opts2 = malloc(strlen("rc.confirmation:no ")
		       + strlen(opts)
		       + 1);
	strcpy(opts2, "rc.confirmation:no ");
	strcat(opts2, opts);

	return task_exec(opts2);
}

static struct json_object *task_exec_json(const char *opts)
{
	struct json_object *o;
	char *str, *cmd;

	cmd = malloc(strlen("rc.json.array=on ") + strlen(opts) + 1);
	strcpy(cmd, "rc.json.array=on ");
	strcat(cmd, opts);

	str = tw_exec(cmd);

	if (str) {
		o = json_tokener_parse(str);
		free(str);
	} else {
		o = NULL;
	}

	free(cmd);

	if (o && is_error(o))
		return NULL;

	return o;
}

char **json_to_tags(struct json_object *jtask)
{
	struct json_object *jtags, *jtag;
	char **tags;
	int n, i;

	jtags = json_object_object_get(jtask, "tags");

	if (!jtags)
		return NULL;


	n = json_object_array_length(jtags);

	tags = malloc((n + 1) * sizeof(char *));

	for (i = 0; i < n; i++) {
		jtag = json_object_array_get_idx(jtags, i);
		tags[i] = strdup(json_object_get_string(jtag));
	}

	tags[n] = NULL;

	return tags;
}

struct task *json_to_task(struct json_object *jtask)
{
	struct task *task;
	const char *urg;
	struct json_object *json;

	task = malloc(sizeof(struct task));

	json = json_object_object_get(jtask, "id");
	task->id = json_object_get_int(json);

	json = json_object_object_get(jtask, "description");
	task->description = strdup(json_object_get_string(json));

	json = json_object_object_get(jtask, "status");
	task->status = strdup(json_object_get_string(json));

	json = json_object_object_get(jtask, "project");
	if (json)
		task->project
			= strdup(json_object_get_string(json));
	else
		task->project = strdup("");

	json = json_object_object_get(jtask, "priority");
	if (json)
		task->priority
			= strdup(json_object_get_string(json));
	else
		task->priority = strdup("");

	json = json_object_object_get(jtask, "uuid");
	task->uuid = strdup(json_object_get_string(json));

	json = json_object_object_get(jtask, "urgency");
	urg = json_object_get_string(json);
	if (urg)
		task->urgency = strdup(urg);
	else
		task->urgency = NULL;

	task->note = note_get(task->uuid);

	json = json_object_object_get(jtask, "entry");
	task->entry = parse_time(json_object_get_string(json));

	json = json_object_object_get(jtask, "due");
	if (json)
		task->due
			= parse_time(json_object_get_string(json));
	else
		task->due = NULL;

	json = json_object_object_get(jtask, "start");
	if (json)
		task->start
			= parse_time(json_object_get_string(json));
	else
		task->start = NULL;

	json = json_object_object_get(jtask, "recur");
	if (json)
		task->recur = strdup(json_object_get_string(json));
	else
		task->recur = NULL;

	task->tags = json_to_tags(jtask);

	return task;
}

struct task **tw_get_all_tasks(const char *status)
{
	int i, n;
	struct json_object *jtasks, *jtask;
	struct task **tasks;
	char *opts;

	opts = malloc(strlen("export status:") + strlen(status) + 1);

	strcpy(opts, "export status:");
	strcat(opts, status);

	jtasks = task_exec_json(opts);
	free(opts);

	if (!jtasks)
		return NULL;

	n = json_object_array_length(jtasks);

	tasks = malloc((n + 1) * sizeof(struct task *));

	for (i = 0; i < n; i++) {
		jtask = json_object_array_get_idx(jtasks, i);

		tasks[i] = json_to_task(jtask);
	}

	tasks[n] = NULL;

	json_object_put(jtasks);

	return tasks;
}

static char *escape(const char *txt)
{
	char *result;
	char *c;

	result = malloc(2*strlen(txt)+1);
	c = result;

	while (*txt) {
		switch (*txt) {
		case '"':
		case '$':
		case '&':
		case '<':
		case '>':
			*c = '\\'; c++;
			*c = *txt;
			break;
		default:
			*c = *txt;
		}
		c++;
		txt++;
	}

	*c = '\0';

	return result;
}

void tw_modify_description(const char *uuid, const char *newdesc)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" modify :\"")
		      + strlen(newdesc)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify \"%s\"", uuid, newdesc);

	tw_exec(opts);

	free(opts);
}

void tw_modify_project(const char *uuid, const char *newproject)
{
	char *str;
	char *opts;

	str = escape(newproject);

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" modify project:\"")
		      + strlen(str)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify project:\"%s\"", uuid, str);

	tw_exec(opts);

	free(str);
	free(opts);
}

void tw_modify_priority(const char *uuid, const char *priority)
{
	char *str;
	char *opts;

	log_fct_enter();

	str = escape(priority);

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" modify priority:\"")
		      + strlen(str)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify priority:\"%s\"", uuid, str);

	tw_exec(opts);

	free(str);
	free(opts);

	log_fct_exit();
}

void tw_add(const char *newdesc, const char *prj, const char *prio)
{
	char *opts, *eprj;

	log_fct_enter();

	eprj = escape(prj);

	opts = malloc(strlen("add")
		      + strlen(" priority:")
		      + 1
		      + strlen(" project:\\\"")
		      + strlen(eprj)
		      + strlen("\\\"")
		      + strlen(" \"")
		      + strlen(newdesc)
		      + strlen("\"")
		      + 1);

	strcpy(opts, "add");

	if (prio && strlen(prio) == 1) {
		strcat(opts, " priority:");
		strcat(opts, prio);
	}

	if (eprj && strlen(prj)) {
		strcat(opts, " project:\\\"");
		strcat(opts, eprj);
		strcat(opts, "\\\"");
	}

	strcat(opts, " \"");
	strcat(opts, newdesc);
	strcat(opts, "\"");

	tw_exec(opts);

	free(opts);
	free(eprj);

	log_fct_exit();
}

void tw_task_done(const char *uuid)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" done")
		      + 1);
	sprintf(opts, " %s done", uuid);

	tw_exec(opts);

	free(opts);
}

void tw_task_start(const char *uuid)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" start")
		      + 1);
	sprintf(opts, " %s start", uuid);

	tw_exec(opts);

	free(opts);
}

void tw_task_stop(const char *uuid)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" stop")
		      + 1);
	sprintf(opts, " %s stop", uuid);

	tw_exec(opts);

	free(opts);
}

void tw_task_remove(const char *uuid)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" delete")
		      + 1);
	sprintf(opts, " %s delete", uuid);

	tw_exec(opts);

	free(opts);
}

static void task_free(struct task *task)
{
	char **tags;

	if (!task)
		return ;

	free(task->description);
	free(task->status);
	free(task->uuid);
	free(task->note);
	free(task->project);
	free(task->priority);
	free(task->urgency);
	free(task->entry);
	free(task->due);
	free(task->start);
	free(task->recur);

	tags = task->tags;
	if (tags) {
		while (*tags) {
			free(*tags);
			tags++;
		}
		free(task->tags);
	}

	free(task);
}

void tw_task_list_free(struct task **tasks)
{
	struct task **cur;

	if (!tasks)
		return ;

	for (cur = tasks; *cur; cur++)
		task_free(*cur);

	free(tasks);
}

static void project_free(struct project *p)
{
	if (!p)
		return ;

	free(p->name);
	free(p);
}

void tw_project_list_free(struct project **prjs)
{
	struct project **cur;

	if (!prjs)
		return ;

	for (cur = prjs; *cur; cur++)
		project_free(*cur);

	free(prjs);
}

static struct project *project_list_get(struct project **prjs, const char *name)
{
	struct project **cur;

	for (cur = prjs; *cur; cur++)
		if (!strcmp((*cur)->name, name))
			return *cur;

	return NULL;
}

static struct project *project_new(const char *name, int count)
{
	struct project *prj;

	prj = malloc(sizeof(struct project));

	prj->name = strdup(name);
	prj->count = count;

	return prj;
}

static int projects_length(struct project **list)
{
	int n;

	if (!list)
		return 0;

	n = 0;
	while (*list) {
		n++;
		list++;
	}

	return n;
}

static struct project **projects_add(struct project **list, void *item)
{
	int n;
	struct project **result;

	n = projects_length(list);

	result = (struct project **)malloc
		((n + 1 + 1) * sizeof(struct project *));

	if (list)
		memcpy(result, list, n * sizeof(struct project *));

	result[n] = item;
	result[n + 1] = NULL;

	return result;
}

struct project **tw_get_projects(struct task **tasks)
{
	struct task **t_cur;
	struct project **prjs, **tmp, *prj;
	const char *prj_name;

	log_fct_enter();

	prjs = malloc(2 * sizeof(struct project *));
	prjs[0] = project_new("ALL", 0);
	prjs[1] = NULL;

	for (t_cur = tasks; *t_cur; t_cur++) {
		prj_name = (*t_cur)->project;
		prj = project_list_get(prjs, prj_name);
		if (prj) {
			prj->count++;
		} else {
			prj = project_new(prj_name, 1);

			tmp = projects_add(prjs, prj);

			free(prjs);
			prjs = tmp;
		}
		prjs[0]->count++;
	}

	log_fct_exit();

	return prjs;
}

void tw_enable_check_version(int e)
{
	check_version_enabled = e;
}
