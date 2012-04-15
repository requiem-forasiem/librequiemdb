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

#ifndef _LIBREQUIEMDB_PLUGIN_FORMAT_H
#define _LIBREQUIEMDB_PLUGIN_FORMAT_H


#include <librequiem/requiem-plugin.h>

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct requiemdb_plugin_format requiemdb_plugin_format_t;


typedef int (*requiemdb_plugin_format_check_schema_version_func_t)(const char *version);

typedef int (*requiemdb_plugin_format_get_alert_idents_func_t)(requiemdb_sql_t *sql, idmef_criteria_t *criteria,
                                                               int limit, int offset, requiemdb_result_idents_order_t order,
                                                               void **res);

typedef int (*requiemdb_plugin_format_get_heartbeat_idents_func_t)(requiemdb_sql_t *sql, idmef_criteria_t *criteria,
                                                                   int limit, int offset, requiemdb_result_idents_order_t order,
                                                                   void **res);

typedef size_t (*requiemdb_plugin_format_get_message_ident_count_func_t)(void *res);
typedef int (*requiemdb_plugin_format_get_next_message_ident_func_t)(void *res, uint64_t *ident);
typedef void (*requiemdb_plugin_format_destroy_message_idents_resource_func_t)(void *res);
typedef int (*requiemdb_plugin_format_get_alert_func_t)(requiemdb_sql_t *sql, uint64_t ident, idmef_message_t **message);
typedef int (*requiemdb_plugin_format_get_heartbeat_func_t)(requiemdb_sql_t *sql, uint64_t ident, idmef_message_t **message);
typedef int (*requiemdb_plugin_format_delete_alert_func_t)(requiemdb_sql_t *sql, uint64_t ident);
typedef ssize_t (*requiemdb_plugin_format_delete_alert_from_list_func_t)(requiemdb_sql_t *sql, uint64_t *idents, size_t size);
typedef ssize_t (*requiemdb_plugin_format_delete_alert_from_result_idents_func_t)(requiemdb_sql_t *sql,
                                                                                  requiemdb_result_idents_t *results);
typedef int (*requiemdb_plugin_format_delete_heartbeat_func_t)(requiemdb_sql_t *sql, uint64_t ident);
typedef ssize_t (*requiemdb_plugin_format_delete_heartbeat_from_list_func_t)(requiemdb_sql_t *sql, uint64_t *idents, size_t size);
typedef ssize_t (*requiemdb_plugin_format_delete_heartbeat_from_result_idents_func_t)(requiemdb_sql_t *sql,
                                                                                      requiemdb_result_idents_t *results);
typedef int (*requiemdb_plugin_format_insert_message_func_t)(requiemdb_sql_t *sql, idmef_message_t *message);
typedef int (*requiemdb_plugin_format_get_values_func_t)(requiemdb_sql_t *sql, requiemdb_path_selection_t *selection,
                                                         idmef_criteria_t *criteria, int distinct, int limit, int offset, void **res);

typedef int (*requiemdb_plugin_format_get_next_values_func_t)(void *res, requiemdb_path_selection_t *selection, idmef_value_t ***values);
typedef void (*requiemdb_plugin_format_destroy_values_resource_func_t)(void *res);



void requiemdb_plugin_format_set_check_schema_version_func(requiemdb_plugin_format_t *plugin,
                                                           requiemdb_plugin_format_check_schema_version_func_t func);

void requiemdb_plugin_format_set_get_alert_idents_func(requiemdb_plugin_format_t *plugin,
                                                       requiemdb_plugin_format_get_alert_idents_func_t func);

void requiemdb_plugin_format_set_get_heartbeat_idents_func(requiemdb_plugin_format_t *plugin,
                                                           requiemdb_plugin_format_get_heartbeat_idents_func_t func);

void requiemdb_plugin_format_set_get_message_ident_count_func(requiemdb_plugin_format_t *plugin,
                                                              requiemdb_plugin_format_get_message_ident_count_func_t func);

void requiemdb_plugin_format_set_get_next_message_ident_func(requiemdb_plugin_format_t *plugin,
                                                             requiemdb_plugin_format_get_next_message_ident_func_t func);

void requiemdb_plugin_format_set_destroy_message_idents_resource_func(requiemdb_plugin_format_t *plugin,
                                                                      requiemdb_plugin_format_destroy_message_idents_resource_func_t func);

void requiemdb_plugin_format_set_get_alert_func(requiemdb_plugin_format_t *plugin, requiemdb_plugin_format_get_alert_func_t func);

void requiemdb_plugin_format_set_get_heartbeat_func(requiemdb_plugin_format_t *plugin, requiemdb_plugin_format_get_heartbeat_func_t func);

void requiemdb_plugin_format_set_delete_alert_func(requiemdb_plugin_format_t *plugin, requiemdb_plugin_format_delete_alert_func_t func);

void requiemdb_plugin_format_set_delete_alert_from_list_func(requiemdb_plugin_format_t *plugin,
                                                             requiemdb_plugin_format_delete_alert_from_list_func_t func);

void requiemdb_plugin_format_set_delete_alert_from_result_idents_func(requiemdb_plugin_format_t *plugin,
                                                                      requiemdb_plugin_format_delete_alert_from_result_idents_func_t func);

ssize_t _requiemdb_plugin_format_delete_alert_from_list(requiemdb_plugin_format_t *plugin,
                                                        requiemdb_sql_t *sql, uint64_t *idents, size_t size);
         
ssize_t _requiemdb_plugin_format_delete_alert_from_result_idents(requiemdb_plugin_format_t *plugin,
                                                                 requiemdb_sql_t *sql, requiemdb_result_idents_t *result);
         
void requiemdb_plugin_format_set_delete_heartbeat_func(requiemdb_plugin_format_t *plugin,
                                                       requiemdb_plugin_format_delete_heartbeat_func_t func);

void requiemdb_plugin_format_set_delete_heartbeat_from_list_func(requiemdb_plugin_format_t *plugin,
                                                                 requiemdb_plugin_format_delete_heartbeat_from_list_func_t func);
         
void requiemdb_plugin_format_set_delete_heartbeat_from_result_idents_func(requiemdb_plugin_format_t *plugin,
                                                                          requiemdb_plugin_format_delete_heartbeat_from_result_idents_func_t func);

ssize_t _requiemdb_plugin_format_delete_heartbeat_from_list(requiemdb_plugin_format_t *plugin,
                                                            requiemdb_sql_t *sql, uint64_t *idents, size_t size);
         
ssize_t _requiemdb_plugin_format_delete_heartbeat_from_result_idents(requiemdb_plugin_format_t *plugin,
                                                                     requiemdb_sql_t *sql, requiemdb_result_idents_t *result);
         
void requiemdb_plugin_format_set_insert_message_func(requiemdb_plugin_format_t *plugin,
                                                     requiemdb_plugin_format_insert_message_func_t func);

void requiemdb_plugin_format_set_get_values_func(requiemdb_plugin_format_t *plugin,
                                                 requiemdb_plugin_format_get_values_func_t func);

void requiemdb_plugin_format_set_get_next_values_func(requiemdb_plugin_format_t *plugin,
                                                      requiemdb_plugin_format_get_next_values_func_t func);


void requiemdb_plugin_format_set_destroy_values_resource_func(requiemdb_plugin_format_t *plugin,
                                                              requiemdb_plugin_format_destroy_values_resource_func_t func);
         
int requiemdb_plugin_format_new(requiemdb_plugin_format_t **ret);

#ifdef __cplusplus
  }
#endif

#endif /* _LIBREQUIEMDB_PLUGIN_FORMAT_H */
