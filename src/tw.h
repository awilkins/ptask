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

#ifndef _PTASK_TW_H_
#define _PTASK_TW_H_

struct task {
	int id;
	char *description;
	char *status;
	char *uuid;
	char *note;
	char *project;
	char *priority;
	char *urgency;
	char *recur;
	char **tags;
	struct tm *entry;
	struct tm *due;
	struct tm *start;
};

struct project {
	char *name;
	int count;
};

struct task **tw_get_all_tasks(const char *status);
void tw_modify_description(const char *uuid, const char *newdesc);
void tw_modify_project(const char *uuid, const char *newproj);
void tw_modify_priority(const char *uuid, const char *priority);
void tw_task_done(const char *uuid);
void tw_task_start(const char *uuid);
void tw_task_stop(const char *uuid);
void tw_task_remove(const char *);
void tw_add(const char *newdesc, const char *prj, const char *prio);
void tw_task_list_free(struct task **tasks);
struct project **tw_get_projects(struct task **tasks);
void tw_project_list_free(struct project **);

/* Whether ptask check that the taskwarrior version is supported. */
void tw_enable_check_version(int e);

#endif
