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

#ifndef _LIBREQUIEMDB_PLUGIN_FORMAT_PRV_H
#define _LIBREQUIEMDB_PLUGIN_FORMAT_PRV_H

#include "requiemdb-plugin-format.h"

struct requiemdb_plugin_format {
	REQUIEM_PLUGIN_GENERIC;

        requiemdb_plugin_format_check_schema_version_func_t check_schema_version;
        requiemdb_plugin_format_get_alert_idents_func_t get_alert_idents;
        requiemdb_plugin_format_get_heartbeat_idents_func_t get_heartbeat_idents;
        requiemdb_plugin_format_get_message_ident_count_func_t get_message_ident_count;
        requiemdb_plugin_format_get_next_message_ident_func_t get_next_message_ident;
        requiemdb_plugin_format_destroy_message_idents_resource_func_t destroy_message_idents_resource;
        requiemdb_plugin_format_get_alert_func_t get_alert;
        requiemdb_plugin_format_get_heartbeat_func_t get_heartbeat;
        requiemdb_plugin_format_delete_alert_func_t delete_alert;
        requiemdb_plugin_format_delete_alert_from_list_func_t delete_alert_from_list;
        requiemdb_plugin_format_delete_alert_from_result_idents_func_t delete_alert_from_result_idents;
        requiemdb_plugin_format_delete_heartbeat_func_t delete_heartbeat;
        requiemdb_plugin_format_delete_heartbeat_from_list_func_t delete_heartbeat_from_list;
        requiemdb_plugin_format_delete_heartbeat_from_result_idents_func_t delete_heartbeat_from_result_idents;
        requiemdb_plugin_format_insert_message_func_t insert_message;
        requiemdb_plugin_format_get_values_func_t get_values;
        requiemdb_plugin_format_get_next_values_func_t get_next_values;
        requiemdb_plugin_format_destroy_values_resource_func_t destroy_values_resource;
};

#endif
