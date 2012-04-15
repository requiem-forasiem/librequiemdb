/*****
*
* Copyright (C) 2005 PreludeIDS Technologies. All Rights Reserved.
* Author: Nicolas Delon 
*
* This file is part of the RequiemDB library.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/

#ifndef _LIBREQUIEMDB_SQL_SETTINGS_H
#define _LIBREQUIEMDB_SQL_SETTINGS_H

#ifdef __cplusplus
 extern "C" {
#endif
         
#define REQUIEMDB_SQL_SETTING_HOST "host"
#define REQUIEMDB_SQL_SETTING_PORT "port"
#define REQUIEMDB_SQL_SETTING_NAME "name"
#define REQUIEMDB_SQL_SETTING_USER "user"
#define REQUIEMDB_SQL_SETTING_PASS "pass"
#define REQUIEMDB_SQL_SETTING_TYPE "type"
#define REQUIEMDB_SQL_SETTING_FILE "file"
#define REQUIEMDB_SQL_SETTING_LOG "log"

typedef struct requiemdb_sql_settings requiemdb_sql_settings_t;


int requiemdb_sql_settings_new(requiemdb_sql_settings_t **settings);
int requiemdb_sql_settings_new_from_string(requiemdb_sql_settings_t **settings, const char *str);

void requiemdb_sql_settings_destroy(requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set(requiemdb_sql_settings_t *settings,
			       const char *name, const char *value);
int requiemdb_sql_settings_set_from_string(requiemdb_sql_settings_t *settings, const char *str);

const char *requiemdb_sql_settings_get(const requiemdb_sql_settings_t *settings, const char *name);

int requiemdb_sql_settings_set_host(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_host(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_port(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_port(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_name(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_name(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_user(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_user(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_pass(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_pass(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_log(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_log(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_type(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_type(const requiemdb_sql_settings_t *settings);

int requiemdb_sql_settings_set_file(requiemdb_sql_settings_t *settings, const char *value);
const char *requiemdb_sql_settings_get_file(const requiemdb_sql_settings_t *settings);

         
#ifdef __cplusplus
  }
#endif

#endif /* _LIBREQUIEMDB_SQL_SETTINGS_H */
