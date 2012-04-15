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
#include <string.h>
#include <stdlib.h>

#include "requiemdb.h"
#include "requiemdb-plugin-sql.h"


#if ! defined(ENOTSUP) && defined(EOPNOTSUPP)
# define ENOTSUP EOPNOTSUPP
#endif

#define REQUIEMDB_ENOTSUP(x) requiemdb_error_verbose(requiem_error_code_from_errno(ENOTSUP), "Database backend does not support '%s' operation", x)


struct requiemdb_plugin_sql {
        REQUIEM_PLUGIN_GENERIC;

        requiemdb_plugin_sql_open_func_t open;
        requiemdb_plugin_sql_close_func_t close;
        requiemdb_plugin_sql_escape_func_t escape;
        requiemdb_plugin_sql_escape_binary_func_t escape_binary;
        requiemdb_plugin_sql_unescape_binary_func_t unescape_binary;
        requiemdb_plugin_sql_query_func_t query;
        requiemdb_plugin_sql_get_column_count_func_t get_column_count;
        requiemdb_plugin_sql_get_row_count_func_t get_row_count;
        requiemdb_plugin_sql_get_column_name_func_t get_column_name;
        requiemdb_plugin_sql_get_column_num_func_t get_column_num;
        requiemdb_plugin_sql_resource_destroy_func_t resource_destroy;
        requiemdb_plugin_sql_fetch_row_func_t fetch_row;
        requiemdb_plugin_sql_fetch_field_func_t fetch_field;
        requiemdb_plugin_sql_build_time_constraint_string_func_t build_time_constraint_string;
        requiemdb_plugin_sql_build_time_interval_string_func_t build_time_interval_string;
        requiemdb_plugin_sql_build_limit_offset_string_func_t build_limit_offset_string;
        requiemdb_plugin_sql_build_constraint_string_func_t build_constraint_string;
        requiemdb_plugin_sql_get_operator_string_func_t get_operator_string;
        requiemdb_plugin_sql_build_timestamp_string_func_t build_timestamp_string;

};



static void bin2hex(const unsigned char *in, size_t inlen, char *out)
{
        static const char hex[] = "0123456789ABCDEF";

        while ( inlen-- ) {
                *(out++) = hex[*in >> 4];
                *(out++) = hex[*in++ & 0x0f];
        }
}



void requiemdb_plugin_sql_set_open_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_open_func_t func)
{
        plugin->open = func;
}


int _requiemdb_plugin_sql_open(requiemdb_plugin_sql_t *plugin,
                               requiemdb_sql_settings_t *settings, void **session)
{
        return plugin->open(settings, session);
}



void requiemdb_plugin_sql_set_close_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_close_func_t func)
{
        plugin->close = func;
}


void _requiemdb_plugin_sql_close(requiemdb_plugin_sql_t *plugin, void *session)
{
        plugin->close(session);
}


void requiemdb_plugin_sql_set_escape_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_escape_func_t func)
{
        plugin->escape = func;
}


int _requiemdb_plugin_sql_escape(requiemdb_plugin_sql_t *plugin, void *session, const char *input, size_t input_size, char **output)
{
        if ( ! plugin->escape )
                return _requiemdb_plugin_sql_escape_binary(plugin, session, (const unsigned char *) input, input_size, output);

        return plugin->escape(session, input, input_size, output);
}


void requiemdb_plugin_sql_set_escape_binary_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_escape_binary_func_t func)
{
        plugin->escape_binary = func;
}


int _requiemdb_plugin_sql_escape_binary(requiemdb_plugin_sql_t *plugin, void *session,
                                        const unsigned char *input, size_t input_size, char **output)
{
        size_t outsize;

        if ( plugin->escape_binary )
                return plugin->escape_binary(session, input, input_size, output);

        outsize = (input_size * 2) + 4;
        if ( outsize <= input_size )
                return requiemdb_error(REQUIEMDB_ERROR_GENERIC);

        *output = malloc(outsize);
        if ( ! *output )
                return requiemdb_error_from_errno(errno);

        (*output)[0] = 'X';
        (*output)[1] = '\'';

        bin2hex(input, input_size, *output + 2);

        (*output)[outsize - 2] = '\'';
        (*output)[outsize - 1] = '\0';

        return 0;
}


void requiemdb_plugin_sql_set_unescape_binary_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_unescape_binary_func_t func)
{
        plugin->unescape_binary = func;
}


int _requiemdb_plugin_sql_unescape_binary(requiemdb_plugin_sql_t *plugin, void *session, const char *input,
                                          size_t input_size, unsigned char **output, size_t *output_size)
{
        if ( plugin->unescape_binary )
                return plugin->unescape_binary(session, input, output, output_size);

        *output = malloc(input_size);
        if ( ! *output )
                return requiemdb_error_from_errno(errno);

        memcpy(*output, input, input_size);
        *output_size = input_size;

        return 0;
}


void requiemdb_plugin_sql_set_query_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_query_func_t func)
{
        plugin->query = func;
}


int _requiemdb_plugin_sql_query(requiemdb_plugin_sql_t *plugin, void *session, const char *query, void **res)
{
        return plugin->query(session, query, res);
}


void requiemdb_plugin_sql_set_get_column_count_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_column_count_func_t func)
{
        plugin->get_column_count = func;
}


unsigned int _requiemdb_plugin_sql_get_column_count(requiemdb_plugin_sql_t *plugin, void *session, void *resource)
{
        return plugin->get_column_count(session, resource);
}


void requiemdb_plugin_sql_set_get_row_count_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_row_count_func_t func)
{
        plugin->get_row_count = func;
}


unsigned int _requiemdb_plugin_sql_get_row_count(requiemdb_plugin_sql_t *plugin, void *session, void *resource)
{
        return plugin->get_row_count(session, resource);
}


void requiemdb_plugin_sql_set_get_column_name_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_column_name_func_t func)
{
        plugin->get_column_name = func;
}


const char *_requiemdb_plugin_sql_get_column_name(requiemdb_plugin_sql_t *plugin, void *session, void *resource, unsigned int column_num)
{
        return plugin->get_column_name(session, resource, column_num);
}


void requiemdb_plugin_sql_set_get_column_num_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_get_column_num_func_t func)
{
        plugin->get_column_num = func;
}


int _requiemdb_plugin_sql_get_column_num(requiemdb_plugin_sql_t *plugin, void *session, void *resource, const char *column_name)
{
        return plugin->get_column_num(session, resource, column_name);
}


void requiemdb_plugin_sql_set_resource_destroy_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_resource_destroy_func_t func)
{
        plugin->resource_destroy = func;
}


void _requiemdb_plugin_sql_resource_destroy(requiemdb_plugin_sql_t *plugin, void *session, void *resource)
{
        plugin->resource_destroy(session, resource);
}


void requiemdb_plugin_sql_set_fetch_row_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_fetch_row_func_t func)
{
        plugin->fetch_row = func;
}


int _requiemdb_plugin_sql_fetch_row(requiemdb_plugin_sql_t *plugin, void *session, void *resource, void **row)
{
        if ( ! plugin->fetch_row )
                REQUIEMDB_ENOTSUP("fetch_row");

        return plugin->fetch_row(session, resource, row);
}


void requiemdb_plugin_sql_set_fetch_field_func(requiemdb_plugin_sql_t *plugin, requiemdb_plugin_sql_fetch_field_func_t func)
{
        plugin->fetch_field = func;
}


int _requiemdb_plugin_sql_fetch_field(requiemdb_plugin_sql_t *plugin,
                                      void *session, void *resource, void *row,
                                      unsigned int column_num, const char **value, size_t *len)
{
        if ( ! plugin->fetch_field )
                return REQUIEMDB_ENOTSUP("fetch_field");

        return plugin->fetch_field(session, resource, row, column_num, value, len);
}


void requiemdb_plugin_sql_set_build_time_constraint_string_func(requiemdb_plugin_sql_t *plugin,
                                                                requiemdb_plugin_sql_build_time_constraint_string_func_t func)
{
        plugin->build_time_constraint_string = func;
}


int _requiemdb_plugin_sql_build_time_constraint_string(requiemdb_plugin_sql_t *plugin,
                                                       requiem_string_t *output, const char *field,
                                                       requiemdb_sql_time_constraint_type_t type,
                                                       idmef_criterion_operator_t operator, int value, int gmt_offset)
{
        if ( ! plugin->build_time_constraint_string )
                return REQUIEMDB_ENOTSUP("build_time_constraint_string");

        return plugin->build_time_constraint_string(output, field, type, operator, value, gmt_offset);
}



void requiemdb_plugin_sql_set_build_time_interval_string_func(requiemdb_plugin_sql_t *plugin,
                                                              requiemdb_plugin_sql_build_time_interval_string_func_t func)
{
        plugin->build_time_interval_string = func;
}


int _requiemdb_plugin_sql_build_time_interval_string(requiemdb_plugin_sql_t *plugin,
                                                     requiemdb_sql_time_constraint_type_t type, int value, char *buf, size_t size)
{
        if ( ! plugin->build_time_interval_string )
                return REQUIEMDB_ENOTSUP("build_time_interval_string");

        return plugin->build_time_interval_string(type, value, buf, size);
}


void requiemdb_plugin_sql_set_build_limit_offset_string_func(requiemdb_plugin_sql_t *plugin,
                                                             requiemdb_plugin_sql_build_limit_offset_string_func_t func)
{
        plugin->build_limit_offset_string = func;
}


int _requiemdb_plugin_sql_build_limit_offset_string(requiemdb_plugin_sql_t *plugin,
                                                    void *session, int limit, int offset, requiem_string_t *output)
{
        if ( ! plugin->build_limit_offset_string )
                return REQUIEMDB_ENOTSUP("build_limit_offset_string");

        return plugin->build_limit_offset_string(session, limit, offset, output);
}


void requiemdb_plugin_sql_set_build_constraint_string_func(requiemdb_plugin_sql_t *plugin,
                                                           requiemdb_plugin_sql_build_constraint_string_func_t func)
{
        plugin->build_constraint_string = func;
}


int _requiemdb_plugin_sql_build_constraint_string(requiemdb_plugin_sql_t *plugin,
                                                  requiem_string_t *out, const char *field,
                                                  idmef_criterion_operator_t operator, const char *value)
{
        if ( ! plugin->build_constraint_string )
                return REQUIEMDB_ENOTSUP("build_constraint_string");

        return plugin->build_constraint_string(out, field, operator, value);
}


void requiemdb_plugin_sql_set_get_operator_string_func(requiemdb_plugin_sql_t *plugin,
                                                       requiemdb_plugin_sql_get_operator_string_func_t func)
{
        plugin->get_operator_string = func;
}

const char *_requiemdb_plugin_sql_get_operator_string(requiemdb_plugin_sql_t *plugin, idmef_criterion_operator_t operator)
{
        return plugin->get_operator_string(operator);
}


void requiemdb_plugin_sql_set_build_timestamp_string_func(requiemdb_plugin_sql_t *plugin,
                                                          requiemdb_plugin_sql_build_timestamp_string_func_t func)
{
        plugin->build_timestamp_string = func;
}


int _requiemdb_plugin_sql_build_timestamp_string(requiemdb_plugin_sql_t *plugin, const struct tm *lt, char *out, size_t size)
{
        int ret;

        if ( plugin->build_timestamp_string )
                return plugin->build_timestamp_string(lt, out, size);

        ret = snprintf(out, size, "'%d-%.2d-%.2d %.2d:%.2d:%.2d'",
                       lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                       lt->tm_hour, lt->tm_min, lt->tm_sec);

        return (ret < 0 || (size_t) ret >= size) ? -1 : 0;
}



int requiemdb_plugin_sql_new(requiemdb_plugin_sql_t **plugin)
{
        *plugin = calloc(1, sizeof(**plugin));
        if ( ! *plugin )
                return requiem_error_from_errno(errno);

        return 0;
}
