/*****
*
* Copyright (C) 2003-2007 PreludeIDS Technologies. All Rights Reserved.
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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <librequiem/requiem-error.h>
#include <librequiem/idmef.h>
#include <libpq-fe.h>

#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb-error.h"
#include "requiemdb-plugin-sql.h"
#include "requiemdb-path-selection.h"
#include "requiemdb.h"


int pgsql_LTX_requiem_plugin_version(void);
int pgsql_LTX_requiemdb_plugin_init(requiem_plugin_entry_t *pe, void *data);


struct pg_result {
        PGresult *result;
        int row;
};



static int handle_error(requiem_error_code_t code, PGconn *conn)
{
        int ret;
        char *tmp;
        size_t len;
        const char *error;

        if ( PQstatus(conn) == CONNECTION_BAD )
                code = REQUIEMDB_ERROR_CONNECTION;

        error = PQerrorMessage(conn);
        if ( ! error )
                return requiemdb_error(code);

        /*
         * Pgsql error message are formatted. Remove trailing '\n'.
         */
        tmp = strdup(error);
        if ( ! tmp )
                return requiemdb_error_verbose(code, "%s", error);

        len = strlen(tmp) - 1;
        while ( tmp[len] == '\n' || tmp[len] == ' ' )
                tmp[len--] = 0;

        ret = requiemdb_error_verbose(code, "%s", tmp);
        free(tmp);

        return ret;
}



static int sql_query(void *session, const char *query, void **resource)
{
        int ret;
        struct pg_result *res;

        res = calloc(1, sizeof (*res));
        if ( ! res )
                return requiem_error_from_errno(errno);

        res->row = -1;

        res->result = PQexec(session, query);
        if ( ! res->result ) {
                free(res);
                return handle_error(REQUIEMDB_ERROR_QUERY, session);
        }

        ret = PQresultStatus(res->result);
        if ( ret == PGRES_TUPLES_OK && PQntuples(res->result) != 0 ) {
                *resource = res;
                return 1;
        }

        PQclear(res->result);
        free(res);
        if ( ret == PGRES_TUPLES_OK || ret == PGRES_COMMAND_OK )
                return 0;

        return handle_error(REQUIEMDB_ERROR_QUERY, session);
}


static int sql_open(requiemdb_sql_settings_t *settings, void **session)
{
        int ret;
        PGconn *conn;

        conn = PQsetdbLogin(requiemdb_sql_settings_get_host(settings),
                            requiemdb_sql_settings_get_port(settings),
                            NULL,
                            NULL,
                            requiemdb_sql_settings_get_name(settings),
                            requiemdb_sql_settings_get_user(settings),
                            requiemdb_sql_settings_get_pass(settings));

        if ( PQstatus(conn) == CONNECTION_BAD ) {
                ret = handle_error(REQUIEMDB_ERROR_CONNECTION, conn);
                PQfinish(conn);
                return ret;
        }

        *session = conn;

        return sql_query(conn, "SET DATESTYLE TO 'ISO'", NULL);
}



static void sql_close(void *session)
{
        PQfinish(session);
}


static int sql_escape(void *session, const char *input, size_t input_size, char **output)
{
        size_t rsize;

        rsize = input_size * 2 + 3;
        if ( rsize <= input_size )
                return requiemdb_error(REQUIEMDB_ERROR_GENERIC);

        *output = malloc(rsize);
        if ( ! *output )
                return requiemdb_error_from_errno(errno);

        (*output)[0] = '\'';

        rsize = PQescapeString((*output) + 1, input, input_size);

        (*output)[rsize + 1] = '\'';
        (*output)[rsize + 2] = '\0';

        return 0;
}


static int sql_escape_binary(void *session, const unsigned char *input, size_t input_size, char **output)
{
        int ret;
        size_t dummy;
        unsigned char *ptr;
        requiem_string_t *string;

        ret = requiem_string_new(&string);
        if ( ret < 0 )
                return ret;

        ptr = PQescapeBytea(input, input_size, &dummy);
        if ( ! ptr ) {
                requiem_string_destroy(string);
                return -1;
        }

#ifdef HAVE_PQSERVERVERSION
        if ( PQserverVersion(session) >= 80200 )
                ret = requiem_string_sprintf(string, "E'%s'", ptr);
        else
#endif
                ret = requiem_string_sprintf(string, "'%s'", ptr);

        free(ptr);

        if ( ret < 0 ) {
                requiem_string_destroy(string);
                return ret;
        }

        ret = requiem_string_get_string_released(string, output);
        requiem_string_destroy(string);

        return 0;
}



static int sql_unescape_binary(void *session, const char *input, unsigned char **output, size_t *output_size)
{
        *output = PQunescapeBytea((const unsigned char *) input, output_size);
        if ( ! *output )
                return requiemdb_error_from_errno(errno);

        return 0;
}



static int sql_build_limit_offset_string(void *session, int limit, int offset, requiem_string_t *output)
{
        if ( limit >= 0 ) {
                if ( offset >= 0 )
                        return requiem_string_sprintf(output, " LIMIT %d OFFSET %d", limit, offset);

                return requiem_string_sprintf(output, " LIMIT %d", limit);
        }

        else if ( offset >= 0 )
                return requiem_string_sprintf(output, " LIMIT ALL OFFSET %d", offset);

        return 0;
}



static void sql_resource_destroy(void *session, void *resource)
{
        if ( resource ) {
                PQclear(((struct pg_result *) resource)->result);
                free(resource);
        }
}



static const char *sql_get_column_name(void *session, void *resource, unsigned int column_num)
{
        return PQfname(((struct pg_result *) resource)->result, column_num);
}



static int sql_get_column_num(void *session, void *resource, const char *column_name)
{
        return PQfnumber(((struct pg_result *) resource)->result, column_name);
}



static unsigned int sql_get_column_count(void *session, void *resource)
{
        return PQnfields(((struct pg_result *) resource)->result);
}



static unsigned int sql_get_row_count(void *session, void *resource)
{
        return PQntuples(((struct pg_result *) resource)->result);
}



static int sql_fetch_row(void *s, void *resource, void **row)
{
        struct pg_result *res = resource;

        /*
         * initialize *row, but we won't use it since we access row's fields directly
         * through the PGresult structure
         */
        *row = NULL;

        if ( res->row + 1 < PQntuples(res->result) ) {
                res->row++;
                return 1;
        }

        return 0;
}



static int sql_fetch_field(void *session, void *resource, void *r,
                           unsigned int column_num, const char **value, size_t *len)
{
        int nfields;
        struct pg_result *res = resource;

        nfields = PQnfields(res->result);
        if ( nfields < 0 || column_num >= (unsigned int) nfields )
                return requiemdb_error(REQUIEMDB_ERROR_INVALID_COLUMN_NUM);

        if ( PQgetisnull(res->result, res->row, column_num) )
                return 0;

        *value = PQgetvalue(res->result, res->row, column_num);
        *len = PQgetlength(res->result, res->row, column_num);

        return 1;
}




static const char *get_operator_string(idmef_criterion_operator_t operator)
{
        int i;
        const struct tbl {
                idmef_criterion_operator_t operator;
                const char *name;
        } tbl[] = {
                { IDMEF_CRITERION_OPERATOR_EQUAL,             "="           },
                { IDMEF_CRITERION_OPERATOR_NOT_EQUAL,         "!="          },

                { IDMEF_CRITERION_OPERATOR_GREATER,           ">"           },
                { IDMEF_CRITERION_OPERATOR_GREATER_OR_EQUAL,  ">="          },
                { IDMEF_CRITERION_OPERATOR_LESSER,            "<"           },
                { IDMEF_CRITERION_OPERATOR_LESSER_OR_EQUAL,   "<="          },

                { IDMEF_CRITERION_OPERATOR_SUBSTR,            "LIKE"        },
                { IDMEF_CRITERION_OPERATOR_SUBSTR_NOCASE,     "ILIKE"       },
                { IDMEF_CRITERION_OPERATOR_NOT_SUBSTR,        "NOT LIKE"    },
                { IDMEF_CRITERION_OPERATOR_NOT_SUBSTR_NOCASE, "NOT ILIKE"   },

                { IDMEF_CRITERION_OPERATOR_REGEX,             "~"           },
                { IDMEF_CRITERION_OPERATOR_REGEX_NOCASE,      "~*"          },
                { IDMEF_CRITERION_OPERATOR_NOT_REGEX,         "!~"          },
                { IDMEF_CRITERION_OPERATOR_NOT_REGEX_NOCASE,  "!~*"         },

                { IDMEF_CRITERION_OPERATOR_NULL,              "IS NULL"     },
                { IDMEF_CRITERION_OPERATOR_NOT_NULL,          "IS NOT NULL" },
                { 0, NULL },
        };

        for ( i = 0; tbl[i].operator != 0; i++ )
                if ( operator == tbl[i].operator )
                        return tbl[i].name;

        return NULL;
}




static int sql_build_constraint_string(requiem_string_t *out, const char *field,
                                       idmef_criterion_operator_t operator, const char *value)
{
        const char *op_str;

        if ( ! value )
                value = "";

        op_str = get_operator_string(operator);
        if ( op_str )
                return requiem_string_sprintf(out, "%s %s %s", field, op_str, value);

        else if ( operator == IDMEF_CRITERION_OPERATOR_EQUAL_NOCASE )
                return requiem_string_sprintf(out, "lower(%s) = lower(%s)", field, value);

        else if ( operator == IDMEF_CRITERION_OPERATOR_NOT_EQUAL_NOCASE )
                return requiem_string_sprintf(out, "lower(%s) != lower(%s)", field, value);

        return -1;
}



static int sql_build_time_constraint_string(requiem_string_t *output, const char *field,
                                            requiemdb_sql_time_constraint_type_t type,
                                            idmef_criterion_operator_t operator, int value, int gmt_offset)
{
        char buf[128];
        const char *sql_operator;
        int ret;

        ret = snprintf(buf, sizeof (buf), "%s + INTERVAL '%d HOUR'", field, gmt_offset / 3600);
        if ( ret < 0 || (size_t) ret >= sizeof (buf) )
                return requiemdb_error(REQUIEMDB_ERROR_GENERIC);

        sql_operator = get_operator_string(operator);
        if ( ! sql_operator )
                return requiemdb_error(REQUIEMDB_ERROR_GENERIC);

        switch ( type ) {
        case REQUIEMDB_SQL_TIME_CONSTRAINT_YEAR:
                return requiem_string_sprintf(output, "EXTRACT(YEAR FROM %s) %s %d",
                                              buf, sql_operator, value);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_MONTH:
                return  requiem_string_sprintf(output, "EXTRACT(MONTH FROM %s) %s %d",
                                               buf, sql_operator, value);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_YDAY:
                return requiem_string_sprintf(output, "EXTRACT(DOY FROM %s) %s %d",
                                              buf, sql_operator, value);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_MDAY:
                return requiem_string_sprintf(output, "EXTRACT(DAY FROM %s) %s %d",
                                              buf, sql_operator, value);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_WDAY:
                return requiem_string_sprintf(output, "EXTRACT(DOW FROM %s) %s %d",
                                              buf, sql_operator, value % 7 + 1);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_HOUR:
                return requiem_string_sprintf(output, "EXTRACT(HOUR FROM %s) %s %d",
                                              buf, sql_operator, value);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_MIN:
                return requiem_string_sprintf(output, "EXTRACT(MINUTE FROM %s) %s %d",
                                              buf, sql_operator, value);

        case REQUIEMDB_SQL_TIME_CONSTRAINT_SEC:
                return requiem_string_sprintf(output, "EXTRACT(SECOND FROM %s) %s %d",
                                              buf, sql_operator, value);
        }

        /* not reached */

        return requiemdb_error(REQUIEMDB_ERROR_GENERIC);
}



static int sql_build_time_interval_string(requiemdb_sql_time_constraint_type_t type, int value,
                                          char *buf, size_t size)
{
        char *type_str;
        int ret;

        switch ( type ) {
        case REQUIEMDB_SQL_TIME_CONSTRAINT_YEAR:
                type_str = "YEAR";
                break;

        case REQUIEMDB_SQL_TIME_CONSTRAINT_MONTH:
                type_str = "MONTH";
                break;

        case REQUIEMDB_SQL_TIME_CONSTRAINT_MDAY:
                type_str = "DAY";
                break;

        case REQUIEMDB_SQL_TIME_CONSTRAINT_HOUR:
                type_str = "HOUR";
                break;

        case REQUIEMDB_SQL_TIME_CONSTRAINT_MIN:
                type_str = "MINUTE";
                break;

        case REQUIEMDB_SQL_TIME_CONSTRAINT_SEC:
                type_str = "SECOND";
                break;

        default:
                return requiemdb_error(REQUIEMDB_ERROR_GENERIC);
        }

        ret = snprintf(buf, size, "INTERVAL '%d %s'", value, type_str);

        return (ret < 0 || (size_t) ret >= size) ? requiemdb_error(REQUIEMDB_ERROR_GENERIC) : 0;
}



int pgsql_LTX_requiemdb_plugin_init(requiem_plugin_entry_t *pe, void *data)
{
        int ret;
        requiemdb_plugin_sql_t *plugin;

        ret = requiemdb_plugin_sql_new(&plugin);
        if ( ret < 0 )
                return ret;

        requiem_plugin_set_name((requiem_plugin_generic_t *) plugin, "PgSQL");
        requiem_plugin_entry_set_plugin(pe, (void *) plugin);

        requiemdb_plugin_sql_set_open_func(plugin, sql_open);
        requiemdb_plugin_sql_set_close_func(plugin, sql_close);
        requiemdb_plugin_sql_set_escape_func(plugin, sql_escape);
        requiemdb_plugin_sql_set_escape_binary_func(plugin, sql_escape_binary);
        requiemdb_plugin_sql_set_unescape_binary_func(plugin, sql_unescape_binary);
        requiemdb_plugin_sql_set_query_func(plugin, sql_query);
        requiemdb_plugin_sql_set_resource_destroy_func(plugin, sql_resource_destroy);
        requiemdb_plugin_sql_set_get_column_count_func(plugin, sql_get_column_count);
        requiemdb_plugin_sql_set_get_row_count_func(plugin, sql_get_row_count);
        requiemdb_plugin_sql_set_get_column_name_func(plugin, sql_get_column_name);
        requiemdb_plugin_sql_set_get_column_num_func(plugin, sql_get_column_num);
        requiemdb_plugin_sql_set_get_operator_string_func(plugin, get_operator_string);
        requiemdb_plugin_sql_set_fetch_row_func(plugin, sql_fetch_row);
        requiemdb_plugin_sql_set_fetch_field_func(plugin, sql_fetch_field);
        requiemdb_plugin_sql_set_build_constraint_string_func(plugin, sql_build_constraint_string);
        requiemdb_plugin_sql_set_build_time_constraint_string_func(plugin, sql_build_time_constraint_string);
        requiemdb_plugin_sql_set_build_time_interval_string_func(plugin, sql_build_time_interval_string);
        requiemdb_plugin_sql_set_build_limit_offset_string_func(plugin, sql_build_limit_offset_string);

        return 0;
}



int pgsql_LTX_requiem_plugin_version(void)
{
        return REQUIEM_PLUGIN_API_VERSION;
}
