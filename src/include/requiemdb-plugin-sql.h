/*****
*
* Copyright (C) 2005 PreludeIDS Technologies. All Rights Reserved.
* Author: Yoann Vandoorselaere <yoannv@gmail.com>
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

#ifndef _LIBREQUIEMDB_PLUGIN_SQL_H
#define _LIBREQUIEMDB_PLUGIN_SQL_H

#include <librequiem/requiem-plugin.h>

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct requiemdb_plugin_sql requiemdb_plugin_sql_t;

typedef int (*requiemdb_plugin_sql_open_func_t)(requiemdb_sql_settings_t *settings, void **session);         
typedef void (*requiemdb_plugin_sql_close_func_t)(void *session);
typedef int (*requiemdb_plugin_sql_escape_func_t)(void *session, const char *input, size_t input_size, char **output);
typedef int (*requiemdb_plugin_sql_escape_binary_func_t)(void *session, const unsigned char *input, size_t input_size, char **output);
typedef int (*requiemdb_plugin_sql_unescape_binary_func_t)(void *session, const char *input, unsigned char **output, size_t *output_size);
typedef int (*requiemdb_plugin_sql_query_func_t)(void *session, const char *query, void **res);
typedef unsigned int (*requiemdb_plugin_sql_get_column_count_func_t)(void *session, void *resource);
typedef unsigned int (*requiemdb_plugin_sql_get_row_count_func_t)(void *session, void *resource);
typedef const char *(*requiemdb_plugin_sql_get_column_name_func_t)(void *session, void *resource, unsigned int column_num);
typedef int (*requiemdb_plugin_sql_get_column_num_func_t)(void *session, void *resource, const char *column_name);
typedef void (*requiemdb_plugin_sql_resource_destroy_func_t)(void *session, void *resource);
typedef int (*requiemdb_plugin_sql_fetch_row_func_t)(void *session, void *resource, void **row);
typedef int (*requiemdb_plugin_sql_fetch_field_func_t)(void *session, void *resource, void *row,
                                                       unsigned int column_num, const char **value, size_t *len);

typedef int (*requiemdb_plugin_sql_build_time_constraint_string_func_t)(requiem_string_t *output, const char *field,
                                                                        requiemdb_sql_time_constraint_type_t type,
                                                                        idmef_criterion_operator_t operator, int value, int gmt_offset);

typedef int (*requiemdb_plugin_sql_build_time_interval_string_func_t)(requiemdb_sql_time_constraint_type_t type, int value,
                                                                      char *buf, size_t size);

typedef int (*requiemdb_plugin_sql_build_limit_offset_string_func_t)(void *session, int limit, int offset, requiem_string_t *output);
typedef int (*requiemdb_plugin_sql_build_constraint_string_func_t)(requiem_string_t *out, const char *field,
                                                                   idmef_criterion_operator_t operator, const char *value);

typedef const char *(*requiemdb_plugin_sql_get_operator_string_func_t)(idmef_criterion_operator_t operator);
typedef int (*requiemdb_plugin_sql_build_timestamp_string_func_t)(const struct tm *t, char *out, size_t size);




void requiemdb_plugin_sql_set_open_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_open_func_t func);

int _requiemdb_plugin_sql_open(requiemdb_plugin_sql_t *plugin,
                               requiemdb_sql_settings_t *settings, void **session);

void requiemdb_plugin_sql_set_close_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_close_func_t func);

void _requiemdb_plugin_sql_close(requiemdb_plugin_sql_t *plugin, void *session);

void requiemdb_plugin_sql_set_escape_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_escape_func_t func);

int _requiemdb_plugin_sql_escape(requiemdb_plugin_sql_t *plugin, void *session, const char *input, size_t input_size, char **output);

void requiemdb_plugin_sql_set_escape_binary_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_escape_binary_func_t func);

int _requiemdb_plugin_sql_escape_binary(requiemdb_plugin_sql_t *plugin, void *session,
                                        const unsigned char *input, size_t input_size, char **output);

void requiemdb_plugin_sql_set_unescape_binary_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_unescape_binary_func_t func);

int _requiemdb_plugin_sql_unescape_binary(requiemdb_plugin_sql_t *plugin, void *session, const char *input,
                                          size_t input_size, unsigned char **output, size_t *output_size);

void requiemdb_plugin_sql_set_query_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_query_func_t func);

int _requiemdb_plugin_sql_query(requiemdb_plugin_sql_t *plugin, void *session, const char *query, void **res);

void requiemdb_plugin_sql_set_get_column_count_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_column_count_func_t func);

unsigned int _requiemdb_plugin_sql_get_column_count(requiemdb_plugin_sql_t *plugin, void *session, void *resource);

void requiemdb_plugin_sql_set_get_row_count_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_row_count_func_t func);

unsigned int _requiemdb_plugin_sql_get_row_count(requiemdb_plugin_sql_t *plugin, void *session, void *resource);

void requiemdb_plugin_sql_set_get_column_name_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_column_name_func_t func);

const char *_requiemdb_plugin_sql_get_column_name(requiemdb_plugin_sql_t *plugin, void *session, void *resource, unsigned int column_num);

void requiemdb_plugin_sql_set_get_column_num_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_column_num_func_t func);

int _requiemdb_plugin_sql_get_column_num(requiemdb_plugin_sql_t *plugin, void *session, void *resource, const char *column_name);

void requiemdb_plugin_sql_set_resource_destroy_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_resource_destroy_func_t func);

void _requiemdb_plugin_sql_resource_destroy(requiemdb_plugin_sql_t *plugin, void *session, void *resource);

void requiemdb_plugin_sql_set_fetch_row_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_fetch_row_func_t func);

int _requiemdb_plugin_sql_fetch_row(requiemdb_plugin_sql_t *plugin, void *session, void *resource, void **row);

void requiemdb_plugin_sql_set_fetch_field_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_fetch_field_func_t func);

int _requiemdb_plugin_sql_fetch_field(requiemdb_plugin_sql_t *plugin,
                                      void *session, void *resource, void *row,
                                      unsigned int column_num, const char **value, size_t *len);

void requiemdb_plugin_sql_set_build_time_constraint_string_func(requiemdb_plugin_sql_t *plugin,
                                                                requiemdb_plugin_sql_build_time_constraint_string_func_t func);

int _requiemdb_plugin_sql_build_time_constraint_string(requiemdb_plugin_sql_t *plugin,
                                                       requiem_string_t *output, const char *field,
                                                       requiemdb_sql_time_constraint_type_t type,
                                                       idmef_criterion_operator_t operator, int value, int gmt_offset);

void requiemdb_plugin_sql_set_build_time_interval_string_func(requiemdb_plugin_sql_t *plugin,
                                                              requiemdb_plugin_sql_build_time_interval_string_func_t func);

int _requiemdb_plugin_sql_build_time_interval_string(requiemdb_plugin_sql_t *plugin,
                                                     requiemdb_sql_time_constraint_type_t type, int value, char *buf, size_t size);

void requiemdb_plugin_sql_set_build_limit_offset_string_func(requiemdb_plugin_sql_t *plugin,
                                                             requiemdb_plugin_sql_build_limit_offset_string_func_t func);

int _requiemdb_plugin_sql_build_limit_offset_string(requiemdb_plugin_sql_t *plugin,
                                                    void *session, int limit, int offset, requiem_string_t *output);

void requiemdb_plugin_sql_set_build_constraint_string_func(requiemdb_plugin_sql_t *plugin,
                                                           requiemdb_plugin_sql_build_constraint_string_func_t func);

int _requiemdb_plugin_sql_build_constraint_string(requiemdb_plugin_sql_t *plugin,
                                                  requiem_string_t *out, const char *field,
                                                  idmef_criterion_operator_t operator, const char *value);

void requiemdb_plugin_sql_set_get_operator_string_func(requiemdb_plugin_sql_t *plugin,
                                                       requiemdb_plugin_sql_get_operator_string_func_t func);

const char *_requiemdb_plugin_sql_get_operator_string(requiemdb_plugin_sql_t *plugin, idmef_criterion_operator_t operator);

void requiemdb_plugin_sql_set_build_timestamp_string_func(requiemdb_plugin_sql_t *plugin,
                                                          requiemdb_plugin_sql_build_timestamp_string_func_t func);

int _requiemdb_plugin_sql_build_timestamp_string(requiemdb_plugin_sql_t *plugin, const struct tm *t, char *out, size_t size);

int requiemdb_plugin_sql_new(requiemdb_plugin_sql_t **plugin);

#ifdef __cplusplus
  }
#endif

#endif /* _LIBREQUIEMDB_PLUGIN_SQL_H */
