/*****
*
* Copyright (C) 2003-2005 PreludeIDS Technologies. All Rights Reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <librequiem/requiem-error.h>
#include <librequiem/idmef.h>
#include <librequiem/idmef-tree-wrap.h>

#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb-error.h"

#include "classic-get.h"

#define db_log(sql) requiem_log(REQUIEM_LOG_ERR, "%s\n", requiem_sql_error(sql))
#define log_memory_exhausted() requiem_log(REQUIEM_LOG_ERR, "memory exhausted !\n")

#define get_(type, name)                                                                        \
static int _get_ ## name (requiemdb_sql_t *sql, requiemdb_sql_row_t *row,                       \
                         int index,                                                             \
                         void *parent, int (*parent_new_child)(void *parent, type **child))     \
{                                                                                               \
        requiemdb_sql_field_t *field;                                                           \
        type *value;                                                                            \
        int ret;                                                                                \
                                                                                                \
        ret = requiemdb_sql_row_fetch_field(row, index, &field);                                \
        if ( ret <= 0 )                                                                         \
                return ret;                                                                     \
                                                                                                \
        ret = parent_new_child(parent, &value);                                                 \
        if ( ret < 0 )                                                                          \
                return ret;                                                                     \
                                                                                                \
        return requiemdb_sql_field_to_ ## name (field, value);                                  \
}

get_(uint8_t, uint8)
get_(uint16_t, uint16)
get_(uint32_t, uint32)
get_(float, float)

#define get_uint8(sql, row, index, parent, parent_new_child) \
        _get_uint8(sql, row, index, parent, (int (*)(void *, uint8_t **)) parent_new_child)

#define get_uint16(sql, row, index, parent, parent_new_child) \
        _get_uint16(sql, row, index, parent, (int (*)(void *, uint16_t **)) parent_new_child)

#define get_uint32(sql, row, index, parent, parent_new_child) \
        _get_uint32(sql, row, index, parent, (int (*)(void *, uint32_t **)) parent_new_child)

#define get_float(sql, row, index, parent, parent_new_child) \
        _get_float(sql, row, index, parent, (int (*)(void *, float **)) parent_new_child)




static int _get_string(requiemdb_sql_t *sql, requiemdb_sql_row_t *row,
                       int index,
                       void *parent, int (*parent_new_child)(void *parent, requiem_string_t **child))
{
        requiemdb_sql_field_t *field;
        requiem_string_t *string;
        int ret;

        ret = requiemdb_sql_row_fetch_field(row, index, &field);
        if ( ret <= 0 )
                return ret;

        ret = parent_new_child(parent, &string);
        if ( ret < 0 )
                return ret;

        ret = requiem_string_set_dup_fast(string,
                                          requiemdb_sql_field_get_value(field),
                                          requiemdb_sql_field_get_len(field));

        return (ret < 0) ? ret : 1;
}


static int _get_string_listed(requiemdb_sql_t *sql, requiemdb_sql_row_t *row,
                              int index,
                              void *parent, int (*parent_new_child)(void *parent, requiem_string_t **child, int pos))
{
        requiemdb_sql_field_t *field;
        requiem_string_t *string;
        int ret;

        ret = requiemdb_sql_row_fetch_field(row, index, &field);
        if ( ret <= 0 )
                return ret;

        ret = parent_new_child(parent, &string, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;

        ret = requiem_string_set_dup_fast(string,
                                          requiemdb_sql_field_get_value(field),
                                          requiemdb_sql_field_get_len(field));

        return (ret < 0) ? ret : 1;
}



static int _get_enum(requiemdb_sql_t *sql, requiemdb_sql_row_t *row,
                     int index,
                     void *parent, int (*parent_new_child)(void *parent, int **child), int (*convert_enum)(const char *))
{
        requiemdb_sql_field_t *field;
        int *enum_val;
        int ret;

        ret = requiemdb_sql_row_fetch_field(row, index, &field);
        if ( ret <= 0 )
                return ret;

        ret = parent_new_child(parent, &enum_val);
        if ( ret < 0 )
                return ret;

        *enum_val = convert_enum(requiemdb_sql_field_get_value(field));

        return 1;
}



static int _get_timestamp(requiemdb_sql_t *sql, requiemdb_sql_row_t *row,
                          int time_index, int gmtoff_index, int usec_index,
                          void *parent, int (*parent_new_child)(void *parent, idmef_time_t **child))
{
        requiemdb_sql_field_t *time_field, *gmtoff_field, *usec_field = NULL;
        const char *tmp;
        int32_t gmtoff;
        uint32_t usec = 0;
        idmef_time_t *time;
        int ret;

        ret = requiemdb_sql_row_fetch_field(row, time_index, &time_field);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_row_fetch_field(row, gmtoff_index, &gmtoff_field);
        if ( ret <= 0 )
                return (ret < 0) ? ret : -1;

        if ( usec_index != -1 ) {
                ret = requiemdb_sql_row_fetch_field(row, usec_index, &usec_field);
                if ( ret <= 0 )
                        return (ret < 0) ? ret : -1;

                ret = requiemdb_sql_field_to_uint32(usec_field, &usec);
                if ( ret < 0 )
                        return ret;
        }

        tmp = requiemdb_sql_field_get_value(time_field);

        ret = requiemdb_sql_field_to_int32(gmtoff_field, &gmtoff);
        if ( ret < 0 )
                return ret;

        ret = parent_new_child(parent, &time);
        if ( ret < 0 )
                return ret;

        return requiemdb_sql_time_from_timestamp(time, tmp, gmtoff, usec);
}

#define get_string(sql, row, index, parent, parent_new_child) \
        _get_string(sql, row, index, parent, (int (*)(void *, requiem_string_t **)) parent_new_child)

#define get_string_listed(sql, row, index, parent, parent_new_child) \
        _get_string_listed(sql, row, index, parent, (int (*)(void *, requiem_string_t **, int)) parent_new_child)

#define get_enum(sql, row, index, parent, parent_new_child, convert_enum) \
        _get_enum(sql, row, index, parent, (int (*)(void *, int **)) parent_new_child, convert_enum)

#define get_timestamp(sql, row, time_index, gmtoff_index, usec_index, parent, parent_new_child) \
        _get_timestamp(sql, row, time_index, gmtoff_index, usec_index, parent, (int (*)(void *, idmef_time_t **)) parent_new_child)



static int get_analyzer_time(requiemdb_sql_t *sql,
                             uint64_t message_ident,
                             char parent_type,
                             void *parent,
                             int (*parent_new_child)(void *parent, idmef_time_t **child))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT time, gmtoff, usec "
                                          "FROM Requiem_AnalyzerTime "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 "",
                                          parent_type, message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = get_timestamp(sql, row, 0, 1, 2, parent, parent_new_child);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_detect_time(requiemdb_sql_t *sql,
                           uint64_t message_ident,
                           idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT time, gmtoff, usec "
                                          "FROM Requiem_DetectTime "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);

        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = get_timestamp(sql, row, 0, 1, 2, alert, idmef_alert_new_detect_time);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_create_time(requiemdb_sql_t *sql,
                           uint64_t message_ident,
                           char parent_type,
                           void *parent,
                           int (*parent_new_child)(void *parent, idmef_time_t **time))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                         "SELECT time, gmtoff, usec "
                                         "FROM Requiem_CreateTime "
                                         "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 "",
                                         parent_type, message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = get_timestamp(sql, row, 0, 1, 2, parent, parent_new_child);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_user_id(requiemdb_sql_t *sql,
                       uint64_t message_ident,
                       char parent_type,
                       int parent_index,
                       int file_index,
                       int file_access_index,
                       void *parent, requiem_bool_t listed,
                       int (*_parent_new_child)(void *, idmef_user_id_t **child))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_user_id_t *user_id;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, type, name, number, tty "
                                          "FROM Requiem_UserId "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND "
                                          "_parent0_index = %d AND _parent1_index = %d AND _parent2_index = %d AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          parent_type, message_ident, parent_index, file_index, file_access_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                if ( listed ) {
                        int (*parent_new_child)(void *parent, idmef_user_id_t **, int) =
                                (int (*)(void *, idmef_user_id_t **, int)) _parent_new_child;
                        ret = parent_new_child(parent, &user_id, IDMEF_LIST_APPEND);
                } else {
                        int (*parent_new_child)(void *parent, idmef_user_id_t **) = _parent_new_child;
                        ret = parent_new_child(parent, &user_id);
                }

                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, user_id, idmef_user_id_new_ident);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 1, user_id, idmef_user_id_new_type, idmef_user_id_type_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, user_id, idmef_user_id_new_name);
                if ( ret < 0 )
                        goto error;

                ret = get_uint32(sql, row, 3, user_id, idmef_user_id_new_number);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 4, user_id, idmef_user_id_new_tty);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_user(requiemdb_sql_t *sql,
                    uint64_t message_ident,
                    char parent_type,
                    int parent_index,
                    void *parent,
                    int (*parent_new_child)(void *parent, idmef_user_t **child))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_user_t *user;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, category "
                                          "FROM Requiem_User "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = parent_new_child(parent, &user);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, user, idmef_user_new_ident);
        if ( ret < 0 )
                goto error;

        ret = get_enum(sql, row, 1, user, idmef_user_new_category, idmef_user_category_to_numeric);
        if ( ret < 0 )
                goto error;

        ret = get_user_id(sql, message_ident, parent_type, parent_index, 0, 0, user,
                          TRUE, (int (*)(void *, idmef_user_id_t **)) idmef_user_new_user_id);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_process_arg(requiemdb_sql_t *sql,
                           uint64_t message_ident,
                           char parent_type,
                           char parent_index,
                           void *parent,
                           int (*parent_new_child)(void *parent, requiem_string_t **child, int pos))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT arg "
                                          "FROM Requiem_ProcessArg "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _index != -1 ORDER BY _index ASC",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = get_string_listed(sql, row, 0, parent, parent_new_child);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_process_env(requiemdb_sql_t *sql,
                           uint64_t message_ident,
                           char parent_type,
                           int parent_index,
                           void *parent,
                           int (*parent_new_child)(void *parent, requiem_string_t **child, int pos))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT env "
                                          "FROM Requiem_ProcessEnv "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _index != -1 ORDER BY _index ASC",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = get_string_listed(sql, row, 0, parent, parent_new_child);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_process(requiemdb_sql_t *sql,
                       uint64_t message_ident,
                       char parent_type,
                       int parent_index,
                       void *parent,
                       int (*parent_new_child)(void *parent, idmef_process_t **child))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_process_t *process;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, name, pid, path "
                                          "FROM Requiem_Process "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = parent_new_child(parent, &process);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, process, idmef_process_new_ident);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 1, process, idmef_process_new_name);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 2, process, idmef_process_new_pid);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 3, process, idmef_process_new_path);
        if ( ret < 0 )
                goto error;

        ret = get_process_arg(sql, message_ident, parent_type, parent_index, process,
                              (int (*)(void *, requiem_string_t **, int)) idmef_process_new_arg);
        if ( ret < 0 )
                goto error;

        ret = get_process_env(sql, message_ident, parent_type, parent_index, process,
                              (int (*)(void *, requiem_string_t **, int)) idmef_process_new_env);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_web_service_arg(requiemdb_sql_t *sql,
                               uint64_t message_ident,
                               char parent_type,
                               int parent_index,
                               idmef_web_service_t *web_service)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT arg "
                                          "FROM Requiem_WebServiceArg "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _index != -1 ORDER BY _index ASC",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = get_string_listed(sql, row, 0, web_service, idmef_web_service_new_arg);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_web_service(requiemdb_sql_t *sql,
                           uint64_t message_ident,
                           char parent_type,
                           int parent_index,
                           idmef_service_t *service)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_web_service_t *web_service;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT url, cgi, http_method "
                                          "FROM Requiem_WebService "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_service_new_web_service(service, &web_service);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, web_service, idmef_web_service_new_url);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 1, web_service, idmef_web_service_new_cgi);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 2, web_service, idmef_web_service_new_http_method);
        if ( ret < 0 )
                goto error;

        ret = get_web_service_arg(sql, message_ident, parent_type, parent_index, web_service);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_snmp_service(requiemdb_sql_t *sql,
                            uint64_t message_ident,
                            char parent_type,
                            int parent_index,
                            idmef_service_t *service)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_snmp_service_t *snmp_service;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT snmp_oid, message_processing_model, security_model, security_name, "
                                          "security_level, context_name, context_engine_id, command "
                                          "FROM Requiem_SnmpService "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_service_new_snmp_service(service, &snmp_service);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, snmp_service, idmef_snmp_service_new_oid);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 1, snmp_service, idmef_snmp_service_new_message_processing_model);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 2, snmp_service, idmef_snmp_service_new_security_model);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 3, snmp_service, idmef_snmp_service_new_security_name);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 4, snmp_service, idmef_snmp_service_new_security_level);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 5, snmp_service, idmef_snmp_service_new_context_name);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 6, snmp_service, idmef_snmp_service_new_context_engine_id);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 7, snmp_service, idmef_snmp_service_new_command);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_service(requiemdb_sql_t *sql,
                       uint64_t message_ident,
                       char parent_type,
                       int parent_index,
                       void *parent,
                       int (*parent_new_child)(void *parent, idmef_service_t **child))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_service_t *service;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, ip_version, name, port, iana_protocol_number, iana_protocol_name, portlist, protocol "
                                          "FROM Requiem_Service "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return 0;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = parent_new_child(parent, &service);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, service, idmef_service_new_ident);
        if ( ret < 0 )
                goto error;

        ret = get_uint8(sql, row, 1, service, idmef_service_new_ip_version);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 2, service, idmef_service_new_name);
        if ( ret < 0 )
                goto error;

        ret = get_uint16(sql, row, 3, service, idmef_service_new_port);
        if ( ret < 0 )
                goto error;

        ret = get_uint8(sql, row, 4, service, idmef_service_new_iana_protocol_number);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 5, service, idmef_service_new_iana_protocol_name);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 6, service, idmef_service_new_portlist);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 7, service, idmef_service_new_protocol);
        if ( ret < 0 )
                goto error;

        ret = get_web_service(sql, message_ident, parent_type, parent_index, service);
        if ( ret < 0 )
                goto error;

        ret = get_snmp_service(sql, message_ident, parent_type, parent_index, service);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_address(requiemdb_sql_t *sql,
                       uint64_t message_ident,
                       char parent_type,
                       int parent_index,
                       void *parent,
                       int (*parent_new_child)(void *parent, idmef_address_t **child, int pos))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_address_t *idmef_address;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, category, vlan_name, vlan_num, address, netmask "
                                          "FROM Requiem_Address "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _index != -1 ORDER BY _index ASC",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = parent_new_child(parent, &idmef_address, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, idmef_address, idmef_address_new_ident);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 1, idmef_address, idmef_address_new_category, idmef_address_category_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, idmef_address, idmef_address_new_vlan_name);
                if ( ret < 0 )
                        goto error;

                ret = get_uint32(sql, row, 3, idmef_address, idmef_address_new_vlan_num);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 4, idmef_address, idmef_address_new_address);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 5, idmef_address, idmef_address_new_netmask);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_node(requiemdb_sql_t *sql,
                    uint64_t message_ident,
                    char parent_type,
                    int parent_index,
                    void *parent,
                    int (*parent_new_child)(void *parent, idmef_node_t **node))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_node_t *node;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, category, location, name "
                                          "FROM Requiem_Node "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d",
                                          parent_type, message_ident, parent_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = parent_new_child(parent, &node);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, node, idmef_node_new_ident);
        if ( ret < 0 )
                goto error;

        ret = get_enum(sql, row, 1, node, idmef_node_new_category, idmef_node_category_to_numeric);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 2, node, idmef_node_new_location);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 3, node, idmef_node_new_name);
        if ( ret < 0 )
                goto error;

        ret = get_address(sql, message_ident, parent_type, parent_index, node,
                          (int (*)(void *, idmef_address_t **, int)) idmef_node_new_address);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_analyzer(requiemdb_sql_t *sql,
                        uint64_t message_ident,
                        char parent_type,
                        void *parent,
                        int (*parent_new_child)(void *parent, idmef_analyzer_t **child, int pos))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_analyzer_t *analyzer;
        int ret;
        int index;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT analyzerid, name, manufacturer, model, version, class, ostype, osversion "
                                          "FROM Requiem_Analyzer "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          parent_type, message_ident);
        if ( ret <= 0 )
                return ret;

        index = 0;
        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {
                ret = parent_new_child(parent, &analyzer, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, analyzer, idmef_analyzer_new_analyzerid);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, analyzer, idmef_analyzer_new_name);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, analyzer, idmef_analyzer_new_manufacturer);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 3, analyzer, idmef_analyzer_new_model);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 4, analyzer, idmef_analyzer_new_version);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 5, analyzer, idmef_analyzer_new_class);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 6, analyzer, idmef_analyzer_new_ostype);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 7, analyzer, idmef_analyzer_new_osversion);
                if ( ret < 0 )
                        goto error;

                ret = get_node(sql, message_ident, parent_type, index, analyzer,
                               (int (*)(void *, idmef_node_t **)) idmef_analyzer_new_node);
                if ( ret < 0 )
                        goto error;

                ret = get_process(sql, message_ident, parent_type, index, analyzer,
                                  (int (*)(void *, idmef_process_t **)) idmef_analyzer_new_process);
                if ( ret < 0 )
                        goto error;

                index++;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_action(requiemdb_sql_t *sql,
                      uint64_t message_ident,
                      idmef_assessment_t *assessment)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_action_t *action;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT category, description "
                                          "FROM Requiem_Action "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_assessment_new_action(assessment, &action, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        return ret;

                ret = get_enum(sql, row, 0, action, idmef_action_new_category, idmef_action_category_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, action, idmef_action_new_description);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_confidence(requiemdb_sql_t *sql,
                          uint64_t message_ident,
                          idmef_assessment_t *assessment)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_confidence_t *confidence;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT rating, confidence "
                                          "FROM Requiem_Confidence "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_assessment_new_confidence(assessment, &confidence);
        if ( ret < 0 )
                goto error;

        ret = get_enum(sql, row, 0, confidence, idmef_confidence_new_rating, idmef_confidence_rating_to_numeric);
        if ( ret < 0 )
                goto error;

        ret = get_float(sql, row, 1, confidence, idmef_confidence_new_confidence);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_impact(requiemdb_sql_t *sql,
                      uint64_t message_ident,
                      idmef_assessment_t *assessment)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_impact_t *impact;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT severity, completion, type, description "
                                          "FROM Requiem_Impact "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_assessment_new_impact(assessment, &impact);
        if ( ret < 0 )
                goto error;

        ret = get_enum(sql, row, 0, impact, idmef_impact_new_severity, idmef_impact_severity_to_numeric);
        if ( ret < 0 )
                goto error;

        ret = get_enum(sql, row, 1, impact, idmef_impact_new_completion, idmef_impact_completion_to_numeric);
        if ( ret < 0 )
                goto error;

        ret = get_enum(sql, row, 2, impact, idmef_impact_new_type, idmef_impact_type_to_numeric);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 3, impact, idmef_impact_new_description);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_assessment(requiemdb_sql_t *sql,
                          uint64_t message_ident,
                          idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        idmef_assessment_t *assessment;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT _message_ident "
                                          "FROM Requiem_Assessment "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        requiemdb_sql_table_destroy(table);

        ret = idmef_alert_new_assessment(alert, &assessment);
        if ( ret < 0 )
                goto error;

        ret = get_impact(sql, message_ident, assessment);
        if ( ret < 0 )
                goto error;

        ret = get_confidence(sql, message_ident, assessment);
        if ( ret < 0 )
                goto error;

        ret = get_action(sql, message_ident, assessment);
        if ( ret < 0 )
                goto error;

 error:
        return ret;
}

static int get_file_access_permission(requiemdb_sql_t *sql,
                                      uint64_t message_ident,
                                      int target_index,
                                      int file_index,
                                      int file_access_index,
                                      idmef_file_access_t *parent)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT permission "
                                          "FROM Requiem_FileAccess_Permission "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d AND "
                                          "_parent1_index = %d AND _parent2_index = %d AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident, target_index, file_index, file_access_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = get_string_listed(sql, row, 0, parent, idmef_file_access_new_permission);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_file_access(requiemdb_sql_t *sql,
                           uint64_t message_ident,
                           int target_index,
                           int file_index,
                           idmef_file_t *file)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        requiemdb_sql_field_t *field;
        idmef_file_access_t *file_access;
        uint32_t file_access_count;
        unsigned int cnt;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT COUNT(*) "
                                          "FROM Requiem_FileAccess "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _parent1_index = %d AND _index != -1",
                                          message_ident, target_index, file_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = requiemdb_sql_row_fetch_field(row, 0, &field);
        if ( ret <= 0 )
                goto error;

        ret = requiemdb_sql_field_to_uint32(field, &file_access_count);
        if ( ret < 0 )
                goto error;

        for ( cnt = 0; cnt < file_access_count; cnt++ ) {

                ret = idmef_file_new_file_access(file, &file_access, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_user_id(sql, message_ident, 'F', target_index, file_index, cnt,
                                  file_access, FALSE, (int (*)(void *, idmef_user_id_t **)) idmef_file_access_new_user_id);
                if ( ret < 0 )
                        goto error;

                ret = get_file_access_permission(sql, message_ident, target_index, file_index, cnt, file_access);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_linkage(requiemdb_sql_t *sql,
                       uint64_t message_ident,
                       int target_index,
                       int file_index,
                       idmef_file_t *file)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_linkage_t *linkage;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT category, name, path "
                                          "FROM Requiem_Linkage "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _parent1_index = %d AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident, target_index, file_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_file_new_linkage(file, &linkage, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 0, linkage, idmef_linkage_new_category, idmef_linkage_category_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, linkage, idmef_linkage_new_name);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, linkage, idmef_linkage_new_path);
                if ( ret < 0 )
                        goto error;
        }

        /* FIXME: file in linkage is not currently supported  */

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_inode(requiemdb_sql_t *sql,
                     uint64_t message_ident,
                     int target_index,
                     int file_index,
                     idmef_file_t *file)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_inode_t *inode;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT change_time, change_time_gmtoff, number, major_device, minor_device, "
                                          "c_major_device, c_minor_device "
                                          "FROM Requiem_Inode "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d AND _parent1_index = %d",
                                          message_ident, target_index, file_index);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_file_new_inode(file, &inode);
        if ( ret < 0 )
                goto error;

        ret = get_timestamp(sql, row, 0, 1, -1, inode, idmef_inode_new_change_time);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 2, inode, idmef_inode_new_number);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 3, inode, idmef_inode_new_major_device);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 4, inode, idmef_inode_new_minor_device);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 5, inode, idmef_inode_new_c_major_device);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 6, inode, idmef_inode_new_c_minor_device);
        if ( ret < 0 )
                goto error;

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}


static int get_checksum(requiemdb_sql_t *sql,
                        uint64_t message_ident,
                        int target_index,
                        int file_index,
                        idmef_file_t *file)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_checksum_t *checksum;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT value, checksum_key, algorithm "
                                          "FROM Requiem_Checksum "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64
                                          " AND _parent0_index = %d AND _parent1_index = %d AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident, target_index, file_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_file_new_checksum(file, &checksum, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, checksum, idmef_checksum_new_value);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, checksum, idmef_checksum_new_key);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 2, checksum, idmef_checksum_new_algorithm, idmef_checksum_algorithm_to_numeric);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}


static int get_file(requiemdb_sql_t *sql,
                    uint64_t message_ident,
                    int target_index,
                    idmef_target_t *target)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_file_t *file = NULL;
        int cnt;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, category, name, path, create_time, create_time_gmtoff, "
                                          "modify_time, modify_time_gmtoff, access_time, "
                                          "access_time_gmtoff, data_size, disk_size, fstype, file_type "
                                          "FROM Requiem_File "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _parent0_index = %d AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident, target_index);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_target_new_file(target, &file, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, file, idmef_file_new_ident);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 1, file, idmef_file_new_category, idmef_file_category_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, file, idmef_file_new_name);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 3, file, idmef_file_new_path);
                if ( ret < 0 )
                        goto error;

                ret = get_timestamp(sql, row, 4, 5, -1, file, idmef_file_new_create_time);
                if ( ret < 0 )
                        goto error;

                ret = get_timestamp(sql, row, 6, 7, -1, file, idmef_file_new_modify_time);
                if ( ret < 0 )
                        goto error;

                ret = get_timestamp(sql, row, 8, 9, -1, file, idmef_file_new_access_time);
                if ( ret < 0 )
                        goto error;

                ret = get_uint32(sql, row, 10, file, idmef_file_new_data_size);
                if ( ret < 0 )
                        goto error;

                ret = get_uint32(sql, row, 11, file, idmef_file_new_disk_size);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 12, file, idmef_file_new_fstype, idmef_file_fstype_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 13, file, idmef_file_new_file_type);
                if ( ret < 0 )
                        goto error;
        }

        file = NULL;
        cnt = 0;
        while ( (file = idmef_target_get_next_file(target, file)) ) {

                ret = get_file_access(sql, message_ident, target_index, cnt, file);
                if ( ret < 0 )
                        goto error;

                ret = get_linkage(sql, message_ident, target_index, cnt, file);
                if ( ret < 0 )
                        goto error;

                ret = get_inode(sql, message_ident, target_index, cnt, file);
                if ( ret < 0 )
                        goto error;

                ret = get_checksum(sql, message_ident, target_index, cnt, file);
                if ( ret < 0 )
                        goto error;

                cnt++;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_source(requiemdb_sql_t *sql,
                      uint64_t message_ident,
                      idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_source_t *source;
        int cnt;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, spoofed, interface "
                                          "FROM Requiem_Source "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_alert_new_source(alert, &source, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, source, idmef_source_new_ident);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 1, source, idmef_source_new_spoofed, idmef_source_spoofed_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, source, idmef_source_new_interface);
                if ( ret < 0 )
                        goto error;
        }

        source = NULL;
        cnt = 0;
        while ( (source = idmef_alert_get_next_source(alert, source)) ) {

                ret = get_node(sql, message_ident, 'S', cnt, source, (int (*)(void *, idmef_node_t **)) idmef_source_new_node);
                if ( ret < 0 )
                        goto error;

                ret = get_user(sql, message_ident, 'S', cnt, source, (int (*)(void *, idmef_user_t **)) idmef_source_new_user);
                if ( ret < 0 )
                        goto error;

                ret = get_process(sql, message_ident, 'S', cnt, source, (int (*)(void *, idmef_process_t **)) idmef_source_new_process);
                if ( ret < 0 )
                        goto error;

                ret = get_service(sql, message_ident, 'S', cnt, source, (int (*)(void *, idmef_service_t **)) idmef_source_new_service);
                if ( ret < 0 )
                        goto error;

                cnt++;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_target(requiemdb_sql_t *sql,
                      uint64_t message_ident,
                      idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_target_t *target;
        int cnt;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, decoy, interface "
                                          "FROM Requiem_Target "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_alert_new_target(alert, &target, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, target, idmef_target_new_ident);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 1, target, idmef_target_new_decoy, idmef_target_decoy_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, target, idmef_target_new_interface);
                if ( ret < 0 )
                        goto error;
        }

        target = NULL;
        cnt = 0;
        while ( (target = idmef_alert_get_next_target(alert, target)) ) {

                ret = get_node(sql, message_ident, 'T', cnt, target, (int (*)(void *, idmef_node_t **)) idmef_target_new_node);
                if ( ret < 0 )
                        goto error;

                ret = get_user(sql, message_ident, 'T', cnt, target, (int (*)(void *, idmef_user_t **)) idmef_target_new_user);
                if ( ret < 0 )
                        goto error;

                ret = get_process(sql, message_ident, 'T', cnt, target, (int (*)(void *, idmef_process_t **)) idmef_target_new_process);
                if ( ret < 0 )
                        goto error;

                ret = get_service(sql, message_ident, 'T', cnt, target, (int (*)(void *, idmef_service_t **)) idmef_target_new_service);
                if ( ret < 0 )
                        goto error;

                ret = get_file(sql, message_ident, cnt, target);
                if ( ret < 0 )
                        goto error;

                cnt++;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_additional_data(requiemdb_sql_t *sql,
                               uint64_t message_ident,
                               char parent_type,
                               void *parent,
                               int (*parent_new_child)(void *, idmef_additional_data_t **, int pos))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_additional_data_t *additional_data;
        idmef_data_t *data;
        requiemdb_sql_field_t *field;
        int ret = 0;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT type, meaning, data "
                                          "FROM Requiem_AdditionalData "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64 " AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          parent_type, message_ident);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = parent_new_child(parent, &additional_data, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 0, additional_data, idmef_additional_data_new_type,
                               idmef_additional_data_type_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, additional_data, idmef_additional_data_new_meaning);
                if ( ret < 0 )
                        goto error;

                ret = requiemdb_sql_row_fetch_field(row, 2, &field);
                if ( ret <= 0 )
                        goto error;

                ret = idmef_additional_data_new_data(additional_data, &data);
                if ( ret < 0 )
                        goto error;

                switch ( idmef_additional_data_get_type(additional_data) ) {
                case IDMEF_ADDITIONAL_DATA_TYPE_BOOLEAN: case IDMEF_ADDITIONAL_DATA_TYPE_BYTE: {
                        uint8_t value;

                        ret = requiemdb_sql_field_to_uint8(field, &value);
                        if ( ret < 0 )
                                ret = idmef_data_set_char_string_dup(data, requiemdb_sql_field_get_value(field));
                        else
                                idmef_data_set_byte(data, value);

                        break;
                }

                case IDMEF_ADDITIONAL_DATA_TYPE_CHARACTER: {
                        uint8_t value;

                        ret = requiemdb_sql_field_to_uint8(field, &value);
                        if ( ret < 0 )
                                ret = idmef_data_set_char_string_dup(data, requiemdb_sql_field_get_value(field));
                        else
                                idmef_data_set_char(data, (char) value);

                        break;
                }

                case IDMEF_ADDITIONAL_DATA_TYPE_DATE_TIME:
                case IDMEF_ADDITIONAL_DATA_TYPE_PORTLIST:
                case IDMEF_ADDITIONAL_DATA_TYPE_STRING:
                case IDMEF_ADDITIONAL_DATA_TYPE_XML:
                        ret = idmef_data_set_char_string_dup(data, requiemdb_sql_field_get_value(field));
                        break;

                case IDMEF_ADDITIONAL_DATA_TYPE_REAL: {
                        float value;

                        ret = requiemdb_sql_field_to_float(field, &value);
                        if ( ret < 0 )
                                ret = idmef_data_set_char_string_dup(data, requiemdb_sql_field_get_value(field));
                        else
                                idmef_data_set_float(data, value);

                        break;
                }

                case IDMEF_ADDITIONAL_DATA_TYPE_INTEGER: {
                        uint32_t value;

                        ret = requiemdb_sql_field_to_uint32(field, &value);
                        if ( ret < 0 )
                                ret = idmef_data_set_char_string_dup(data, requiemdb_sql_field_get_value(field));
                        else
                                idmef_data_set_uint32(data, value);

                        break;
                }

                case IDMEF_ADDITIONAL_DATA_TYPE_BYTE_STRING: {
                        unsigned char *value;
                        size_t value_size;

                        ret = requiemdb_sql_unescape_binary(sql,
                                                            requiemdb_sql_field_get_value(field),
                                                            requiemdb_sql_field_get_len(field),
                                                            &value, &value_size);
                        if ( ret < 0 )
                                break;

                        ret = idmef_data_set_byte_string_nodup(data, value, value_size);

                        break;
                }

                case IDMEF_ADDITIONAL_DATA_TYPE_NTPSTAMP: {
                        uint64_t value;

                        ret = requiemdb_sql_field_to_uint64(field, &value);
                        if ( ret < 0 )
                                ret = idmef_data_set_char_string_dup(data, requiemdb_sql_field_get_value(field));
                        else
                                idmef_data_set_uint64(data, value);

                        break;
                }

                case IDMEF_ADDITIONAL_DATA_TYPE_ERROR:
                        ret = -1;
                }

                if ( ret < 0 )
                        goto error;

        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_reference(requiemdb_sql_t *sql,
                         uint64_t message_ident,
                         idmef_classification_t *classification)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_reference_t *reference;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT origin, name, url, meaning "
                                          "FROM Requiem_Reference "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 " AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = idmef_classification_new_reference(classification, &reference, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_enum(sql, row, 0, reference, idmef_reference_new_origin,
                               idmef_reference_origin_to_numeric);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, reference, idmef_reference_new_name);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 2, reference, idmef_reference_new_url);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 3, reference, idmef_reference_new_meaning);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_classification(requiemdb_sql_t *sql,
                              uint64_t message_ident,
                              idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_classification_t *classification;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT ident, text "
                                          "FROM Requiem_Classification "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_alert_new_classification(alert, &classification);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, classification, idmef_classification_new_ident);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 1, classification, idmef_classification_new_text);
        if ( ret < 0 )
                goto error;

        ret = get_reference(sql, message_ident, classification);
        if ( ret < 0 )
                goto error;

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_alertident(requiemdb_sql_t *sql,
                          uint64_t message_ident,
                          char parent_type,
                          void *parent,
                          int (*parent_new_child)(void *parent, idmef_alertident_t **child, int pos))
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_alertident_t *alertident = NULL;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT alertident, analyzerid "
                                          "FROM Requiem_Alertident "
                                          "WHERE _parent_type = '%c' AND _message_ident = %" REQUIEM_PRIu64" AND _index != -1 "
                                          "ORDER BY _index ASC",
                                          parent_type, message_ident);
        if ( ret <= 0 )
                return ret;

        while ( (ret = requiemdb_sql_table_fetch_row(table, &row)) > 0 ) {

                ret = parent_new_child(parent, &alertident, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 0, alertident, idmef_alertident_new_alertident);
                if ( ret < 0 )
                        goto error;

                ret = get_string(sql, row, 1, alertident, idmef_alertident_new_analyzerid);
                if ( ret < 0 )
                        goto error;
        }

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_tool_alert(requiemdb_sql_t *sql,
                          uint64_t message_ident,
                          idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_tool_alert_t *tool_alert;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT name, command "
                                          "FROM Requiem_ToolAlert "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_alert_new_tool_alert(alert, &tool_alert);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, tool_alert, idmef_tool_alert_new_name);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 1, tool_alert, idmef_tool_alert_new_command);
        if ( ret < 0 )
                goto error;

        ret = get_alertident(sql, message_ident, 'T', tool_alert,
                             (int (*)(void *, idmef_alertident_t **, int)) idmef_tool_alert_new_alertident);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}

static int get_correlation_alert(requiemdb_sql_t *sql,
                                 uint64_t message_ident,
                                 idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_correlation_alert_t *correlation_alert;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT name "
                                          "FROM Requiem_CorrelationAlert "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_alert_new_correlation_alert(alert, &correlation_alert);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, correlation_alert, idmef_correlation_alert_new_name);
        if ( ret < 0 )
                goto error;

        ret = get_alertident(sql, message_ident, 'C', correlation_alert,
                             (int (*)(void *, idmef_alertident_t **, int)) idmef_correlation_alert_new_alertident);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}


static int get_overflow_alert(requiemdb_sql_t *sql,
                              uint64_t message_ident,
                              idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        idmef_overflow_alert_t *overflow_alert;
        requiemdb_sql_field_t *field;
        idmef_data_t *buffer;
        unsigned char *data;
        size_t data_size;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table,
                                          "SELECT program, size, buffer "
                                          "FROM Requiem_OverflowAlert "
                                          "WHERE _message_ident = %" REQUIEM_PRIu64 "",
                                          message_ident);
        if ( ret <= 0 )
                return ret;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret <= 0 )
                goto error;

        ret = idmef_alert_new_overflow_alert(alert, &overflow_alert);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, overflow_alert, idmef_overflow_alert_new_program);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 1, overflow_alert, idmef_overflow_alert_new_size);
        if ( ret < 0 )
                goto error;

        ret = requiemdb_sql_row_fetch_field(row, 2, &field);
        if ( ret < 0 )
                goto error;

        ret = idmef_overflow_alert_new_buffer(overflow_alert, &buffer);
        if ( ret < 0 )
                goto error;

        ret = requiemdb_sql_unescape_binary(sql,
                                            requiemdb_sql_field_get_value(field),
                                            requiemdb_sql_field_get_len(field),
                                            &data, &data_size);

        if ( ret < 0 )
                goto error;

        ret = idmef_data_set_byte_string_nodup(buffer, data, data_size);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;
}


static int get_alert_messageid(requiemdb_sql_t *sql, uint64_t ident, idmef_alert_t *alert)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table, "SELECT messageid FROM Requiem_Alert WHERE _ident = %" REQUIEM_PRIu64 "", ident);
        if ( ret < 0 )
                return ret;

        if ( ret == 0 )
                return requiemdb_error(REQUIEMDB_ERROR_INVALID_MESSAGE_IDENT);

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, alert, idmef_alert_new_messageid);

 error:
        requiemdb_sql_table_destroy(table);

        return (ret < 0) ? ret : 1;
}


int classic_get_alert(requiemdb_sql_t *sql, uint64_t ident, idmef_message_t **message)
{
        idmef_alert_t *alert;
        int ret;

        ret = idmef_message_new(message);
        if ( ret < 0 )
                return ret;

        ret = idmef_message_new_alert(*message, &alert);
        if ( ret < 0 )
                goto error;

        ret = get_alert_messageid(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_assessment(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_analyzer(sql, ident, 'A', alert, (int (*)(void *, idmef_analyzer_t **, int)) idmef_alert_new_analyzer);
        if ( ret < 0 )
                goto error;

        ret = get_create_time(sql, ident, 'A', alert, (int (*)(void *, idmef_time_t **)) idmef_alert_new_create_time);
        if ( ret < 0 )
                goto error;

        ret = get_detect_time(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_analyzer_time(sql, ident, 'A', alert, (int (*)(void *, idmef_time_t **)) idmef_alert_new_analyzer_time);
        if ( ret < 0 )
                goto error;

        ret = get_source(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_target(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_classification(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_additional_data(sql, ident, 'A', alert,
                                  (int (*)(void *, idmef_additional_data_t **, int)) idmef_alert_new_additional_data);
        if ( ret < 0 )
                goto error;

        ret = get_tool_alert(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_correlation_alert(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        ret = get_overflow_alert(sql, ident, alert);
        if ( ret < 0 )
                goto error;

        return 0;

 error:
        idmef_message_destroy(*message);

        return ret;
}



static int _get_heartbeat(requiemdb_sql_t *sql, uint64_t ident, idmef_heartbeat_t *heartbeat)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        int ret;

        ret = requiemdb_sql_query_sprintf(sql, &table, "SELECT messageid, heartbeat_interval FROM Requiem_Heartbeat WHERE _ident = %" REQUIEM_PRIu64 "", ident);
        if ( ret < 0 )
                return ret;

        if ( ret == 0 )
                return requiemdb_error(REQUIEMDB_ERROR_INVALID_MESSAGE_IDENT);

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret < 0 )
                goto error;

        ret = get_string(sql, row, 0, heartbeat, idmef_heartbeat_new_messageid);
        if ( ret < 0 )
                goto error;

        ret = get_uint32(sql, row, 1, heartbeat, idmef_heartbeat_new_heartbeat_interval);

 error:
        requiemdb_sql_table_destroy(table);

        return (ret < 0) ? ret : 1;
}



int classic_get_heartbeat(requiemdb_sql_t *sql, uint64_t ident, idmef_message_t **message)
{
        idmef_heartbeat_t *heartbeat;
        int ret;

        ret = idmef_message_new(message);
        if ( ret < 0 )
                return ret;

        ret = idmef_message_new_heartbeat(*message, &heartbeat);
        if ( ret < 0 )
                goto error;

        ret = _get_heartbeat(sql, ident, heartbeat);
        if ( ret <= 0 )
                goto error;

        ret = get_analyzer(sql, ident, 'H', heartbeat, (int (*)(void *, idmef_analyzer_t **, int)) idmef_heartbeat_new_analyzer);
        if ( ret < 0 )
                goto error;

        ret = get_create_time(sql, ident, 'H', heartbeat, (int (*)(void *, idmef_time_t **)) idmef_heartbeat_new_create_time);
        if ( ret < 0 )
                goto error;

        ret = get_analyzer_time(sql, ident, 'H', heartbeat, (int (*)(void *, idmef_time_t **)) idmef_heartbeat_new_analyzer_time);
        if ( ret < 0 )
                goto error;

        ret = get_additional_data(sql, ident, 'H', heartbeat,
                                  (int (*)(void *, idmef_additional_data_t **, int)) idmef_heartbeat_new_additional_data);
        if ( ret < 0 )
                goto error;

        return 0;

 error:
        idmef_message_destroy(*message);

        return ret;
}
