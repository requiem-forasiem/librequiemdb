/*****
*
* Copyright (C) 2005,2006,2007 PreludeIDS Technologies. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>

#include "requiemdb.h"
#include "requiemdb-plugin-format-prv.h"


void requiemdb_plugin_format_set_check_schema_version_func(requiemdb_plugin_format_t *plugin,
                                                           requiemdb_plugin_format_check_schema_version_func_t func)
{
        plugin->check_schema_version = func;
}


void requiemdb_plugin_format_set_get_alert_idents_func(requiemdb_plugin_format_t *plugin,
                                                       requiemdb_plugin_format_get_alert_idents_func_t func)
{
        plugin->get_alert_idents = func;
}


void requiemdb_plugin_format_set_get_heartbeat_idents_func(requiemdb_plugin_format_t *plugin,
                                                           requiemdb_plugin_format_get_heartbeat_idents_func_t func)
{
        plugin->get_heartbeat_idents = func;
}



void requiemdb_plugin_format_set_get_message_ident_count_func(requiemdb_plugin_format_t *plugin,
                                                              requiemdb_plugin_format_get_message_ident_count_func_t func)
{
        plugin->get_message_ident_count = func;
}


void requiemdb_plugin_format_set_get_next_message_ident_func(requiemdb_plugin_format_t *plugin,
                                                             requiemdb_plugin_format_get_next_message_ident_func_t func)
{
        plugin->get_next_message_ident = func;
}


void requiemdb_plugin_format_set_destroy_message_idents_resource_func(requiemdb_plugin_format_t *plugin,
                                                                      requiemdb_plugin_format_destroy_message_idents_resource_func_t func)
{
        plugin->destroy_message_idents_resource = func;
}


void requiemdb_plugin_format_set_get_alert_func(requiemdb_plugin_format_t *plugin, requiemdb_plugin_format_get_alert_func_t func)
{
        plugin->get_alert = func;
}



void requiemdb_plugin_format_set_get_heartbeat_func(requiemdb_plugin_format_t *plugin, requiemdb_plugin_format_get_heartbeat_func_t func)
{
        plugin->get_heartbeat = func;
}


void requiemdb_plugin_format_set_delete_alert_func(requiemdb_plugin_format_t *plugin, requiemdb_plugin_format_delete_alert_func_t func)
{
        plugin->delete_alert = func;
}


void requiemdb_plugin_format_set_delete_alert_from_list_func(requiemdb_plugin_format_t *plugin,
                                                             requiemdb_plugin_format_delete_alert_from_list_func_t func)
{
        plugin->delete_alert_from_list = func;
}


void requiemdb_plugin_format_set_delete_alert_from_result_idents_func(requiemdb_plugin_format_t *plugin,
                                                                      requiemdb_plugin_format_delete_alert_from_result_idents_func_t func)
{
        plugin->delete_alert_from_result_idents = func;
}


void requiemdb_plugin_format_set_delete_heartbeat_func(requiemdb_plugin_format_t *plugin,
                                                       requiemdb_plugin_format_delete_heartbeat_func_t func)
{
        plugin->delete_heartbeat = func;
}


void requiemdb_plugin_format_set_delete_heartbeat_from_list_func(requiemdb_plugin_format_t *plugin,
                                                                 requiemdb_plugin_format_delete_heartbeat_from_list_func_t func)
{
        plugin->delete_heartbeat_from_list = func;
}


void requiemdb_plugin_format_set_delete_heartbeat_from_result_idents_func(requiemdb_plugin_format_t *plugin,
                                                                          requiemdb_plugin_format_delete_heartbeat_from_result_idents_func_t func)
{
        plugin->delete_heartbeat_from_result_idents = func;
}


void requiemdb_plugin_format_set_insert_message_func(requiemdb_plugin_format_t *plugin,
                                                     requiemdb_plugin_format_insert_message_func_t func)
{
        plugin->insert_message = func;
}



void requiemdb_plugin_format_set_get_values_func(requiemdb_plugin_format_t *plugin,
                                                 requiemdb_plugin_format_get_values_func_t func)
{
        plugin->get_values = func;
}


void requiemdb_plugin_format_set_get_next_values_func(requiemdb_plugin_format_t *plugin,
                                                      requiemdb_plugin_format_get_next_values_func_t func)
{
        plugin->get_next_values = func;
}


void requiemdb_plugin_format_set_destroy_values_resource_func(requiemdb_plugin_format_t *plugin,
                                                              requiemdb_plugin_format_destroy_values_resource_func_t func)
{
        plugin->destroy_values_resource = func;
}



int requiemdb_plugin_format_new(requiemdb_plugin_format_t **ret)
{
        *ret = calloc(1, sizeof(**ret));
        if ( ! *ret )
                return requiemdb_error_from_errno(errno);

        return 0;
}



ssize_t _requiemdb_plugin_format_delete_alert_from_list(requiemdb_plugin_format_t *plugin,
                                                        requiemdb_sql_t *sql, uint64_t *idents, size_t size)
{
        size_t i;
        int ret = 0;

        if ( plugin->delete_alert_from_list )
                return plugin->delete_alert_from_list(sql, idents, size);

        for ( i = 0; i < size; i++ ) {
                ret = plugin->delete_alert(sql, idents[i]);
                if ( ret < 0 )
                        return ret;
        }

        return i;
}



ssize_t _requiemdb_plugin_format_delete_alert_from_result_idents(requiemdb_plugin_format_t *plugin,
                                                                 requiemdb_sql_t *sql, requiemdb_result_idents_t *result)
{
        int ret;
        uint64_t ident;
        size_t count = 0;

        if ( plugin->delete_alert_from_result_idents )
                return plugin->delete_alert_from_result_idents(sql, result);

        while ( (ret = requiemdb_result_idents_get_next(result, &ident)) ) {

                ret = plugin->delete_alert(sql, ident);
                if ( ret < 0 )
                        return ret;

                count++;
        }

        return count;
}



ssize_t _requiemdb_plugin_format_delete_heartbeat_from_list(requiemdb_plugin_format_t *plugin,
                                                            requiemdb_sql_t *sql, uint64_t *idents, size_t size)
{
        size_t i;
        int ret = 0;

        if ( plugin->delete_heartbeat_from_list )
                return plugin->delete_heartbeat_from_list(sql, idents, size);

        for ( i = 0; i < size; i++ ) {
                ret = plugin->delete_heartbeat(sql, idents[i]);
                if ( ret < 0 )
                        return ret;
        }

        return i;
}



ssize_t _requiemdb_plugin_format_delete_heartbeat_from_result_idents(requiemdb_plugin_format_t *plugin,
                                                                     requiemdb_sql_t *sql, requiemdb_result_idents_t *result)
{
        int ret;
        uint64_t ident;
        size_t count = 0;

        if ( plugin->delete_heartbeat_from_result_idents )
                return plugin->delete_heartbeat_from_result_idents(sql, result);

        while ( (ret = requiemdb_result_idents_get_next(result, &ident)) ) {

                ret = plugin->delete_heartbeat(sql, ident);
                if ( ret < 0 )
                        return ret;

                count++;
        }

        return count;
}
