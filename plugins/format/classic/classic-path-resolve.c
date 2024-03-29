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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <librequiem/requiem.h>

#include "requiemdb-path-selection.h"
#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"

#include "classic-sql-join.h"
#include "classic-sql-select.h"
#include "classic-path-resolve.h"

#define FIELD_CONTEXT_WHERE    1
#define FIELD_CONTEXT_SELECT   2
#define FIELD_CONTEXT_FUNCTION 3



static int default_table_name_resolver(const idmef_path_t *path, char **table_name)
{
        char c;
        int ret;
        const char *class_name;
        requiem_string_t *string;
        requiem_bool_t next_is_maj = TRUE;

        class_name = idmef_class_get_name(idmef_path_get_class(path, idmef_path_get_depth(path) - 2));

        ret = requiem_string_new(&string);
        if ( ret < 0 )
                return ret;

        ret = requiem_string_cat(string, "Requiem_");
        if ( ret < 0 )
                goto error;

        while ( *class_name ) {
                c = *class_name++;
                if ( c == '_' ) {
                        next_is_maj = TRUE;
                        continue;
                }

                if ( next_is_maj ) {
                        c += 'A' - 'a';
                        next_is_maj = FALSE;
                }

                ret = requiem_string_ncat(string, &c, 1);
                if ( ret < 0 )
                        goto error;
        }

        ret = requiem_string_get_string_released(string, table_name);

 error:
        requiem_string_destroy(string);

        return ret;
}



static int default_field_name_resolver(const idmef_path_t *path, int field_context,
                                const char *table_name, requiem_string_t *output)
{
        return requiem_string_sprintf(output, "%s.%s", table_name,
                                      idmef_path_get_name(path, idmef_path_get_depth(path) - 1));
}



static int time_with_usec_field_name_resolver(const idmef_path_t *path, int field_context,
                                              const char *table_name, requiem_string_t *output)
{
        if ( field_context == FIELD_CONTEXT_SELECT )
                return requiem_string_sprintf(output, "%s.time, %s.gmtoff, %s.usec", table_name, table_name, table_name);

        return requiem_string_sprintf(output, "%s.time", table_name);
}



static int time_without_usec_field_name_resolver(const idmef_path_t *path, int field_context,
                                                 const char *table_name, requiem_string_t *output)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( field_context == FIELD_CONTEXT_SELECT )
                return requiem_string_sprintf(output, "%s.%s, %s.%s_gmtoff, 0",
                                              table_name, child_name, table_name, child_name);

        return requiem_string_sprintf(output, "%s.%s", table_name, child_name);
}



static int message_table_name_resolver(const idmef_path_t *path, char **table_name)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "create_time") == 0 )
                *table_name = strdup("Requiem_CreateTime");
        else if ( strcmp(child_name, "detect_time") == 0 )
                *table_name = strdup("Requiem_DetectTime");
        else if ( strcmp(child_name, "analyzer_time") == 0 )
                *table_name = strdup("Requiem_AnalyzerTime");
        else
                return default_table_name_resolver(path, table_name);

        return *table_name ? 0 : requiem_error_from_errno(errno);
}



static int message_field_name_resolver(const idmef_path_t *path, int field_context, const char *table_name,
                                       requiem_string_t *output)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "create_time") == 0 ||
             strcmp(child_name, "detect_time") == 0 ||
             strcmp(child_name, "analyzer_time") == 0 )
                return time_with_usec_field_name_resolver(path, field_context, table_name, output);

        return default_field_name_resolver(path, field_context, table_name, output);
}



static int process_table_name_resolver(const idmef_path_t *path, char **table_name)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "arg") == 0 )
                *table_name = strdup("Requiem_ProcessArg");
        else if ( strcmp(child_name, "env") == 0 )
                *table_name = strdup("Requiem_ProcessEnv");
        else
                *table_name = strdup("Requiem_Process");

        return *table_name ? 0 : requiem_error_from_errno(errno);
}



static int file_access_table_name_resolver(const idmef_path_t *path, char **table_name)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "permission") == 0 )
                *table_name = strdup("Requiem_FileAccess_Permission");
        else
                *table_name = strdup("Requiem_FileAccess");

        return *table_name ? 0 : requiem_error_from_errno(errno);
}



static int web_service_table_name_resolver(const idmef_path_t *path, char **table_name)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "arg") == 0 )
                *table_name = strdup("Requiem_WebServiceArg");
        else
                *table_name = strdup("Requiem_WebService");

        return *table_name ? 0 : requiem_error_from_errno(errno);
}



static int snmp_field_name_resolver(const idmef_path_t *path, int field_context, const char *table_name,
                                    requiem_string_t *output)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "oid") == 0 )
                child_name = "snmp_oid";

        return requiem_string_sprintf(output, "%s.%s", table_name, child_name);
}



static int checksum_field_name_resolver(const idmef_path_t *path, int field_context, const char *table_name,
                                        requiem_string_t *output)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "key") == 0 )
                child_name = "checksum_key";

        return requiem_string_sprintf(output, "%s.%s", table_name, child_name);
}



static int file_field_name_resolver(const idmef_path_t *path, int field_context, const char *table_name,
                                    requiem_string_t *output)
{
        const char *child_name = idmef_path_get_name(path, idmef_path_get_depth(path) - 1);

        if ( strcmp(child_name, "create_time") == 0 ||
             strcmp(child_name, "modify_time") == 0 ||
             strcmp(child_name, "access_time") == 0 )
                return time_without_usec_field_name_resolver(path, field_context, table_name, output);

        return requiem_string_sprintf(output, "%s.%s", table_name, child_name);
}


typedef struct classic_idmef_class {
        idmef_class_id_t id;
        int (*resolve_table_name)(const idmef_path_t *path, char **table_name);
        int (*resolve_field_name)(const idmef_path_t *path, int field_context,
                                  const char *table_name, requiem_string_t *output);
} classic_idmef_class_t;


static const classic_idmef_class_t classes[] = {
        { IDMEF_CLASS_ID_ALERT, message_table_name_resolver, message_field_name_resolver },
        { IDMEF_CLASS_ID_HEARTBEAT, message_table_name_resolver, message_field_name_resolver },
        { IDMEF_CLASS_ID_PROCESS, process_table_name_resolver, default_field_name_resolver },
        { IDMEF_CLASS_ID_SNMP_SERVICE, default_table_name_resolver, snmp_field_name_resolver },
        { IDMEF_CLASS_ID_WEB_SERVICE, web_service_table_name_resolver, default_field_name_resolver },
        { IDMEF_CLASS_ID_CHECKSUM, default_table_name_resolver, checksum_field_name_resolver },
        { IDMEF_CLASS_ID_FILE, default_table_name_resolver, file_field_name_resolver },
        { IDMEF_CLASS_ID_FILE_ACCESS, file_access_table_name_resolver, default_field_name_resolver },
};



static const classic_idmef_class_t default_class = {
        0,
        default_table_name_resolver,
        default_field_name_resolver
};



static const classic_idmef_class_t *search_path(const idmef_path_t *path)
{
        idmef_class_id_t class_id;
        unsigned int i;

        class_id = idmef_path_get_class(path, idmef_path_get_depth(path) - 2);

        for ( i = 0; i < sizeof (classes) / sizeof (classes[0]); i++ )
                if ( class_id == classes[i].id )
                        return &classes[i];

        return &default_class;
}



static int classic_path_resolve(const idmef_path_t *path, int field_context,
                                classic_sql_join_t *join, requiem_string_t *output)
{
        const classic_idmef_class_t *class;
        classic_sql_joined_table_t *table;
        char *table_name;
        int ret;

        if ( idmef_path_get_depth(path) == 2 && idmef_path_get_value_type(path, 1) != IDMEF_VALUE_TYPE_TIME )
                return default_field_name_resolver(path, field_context, "top_table", output);

        class = search_path(path);

        table = classic_sql_join_lookup_table(join, path);

        if ( ! table ) {
                ret = class->resolve_table_name(path, &table_name);
                if ( ret < 0 )
                        return ret;

                ret = classic_sql_join_new_table(join, &table, path, table_name);
                if ( ret < 0 )
                        return ret;
        }

        return class->resolve_field_name(path, field_context,
                                         classic_sql_joined_table_get_name(table), output);
}



int classic_path_resolve_selected(requiemdb_sql_t *sql,
                                  requiemdb_selected_path_t *selected,
                                  classic_sql_join_t *join, classic_sql_select_t *select)
{
        idmef_path_t *path;
        requiemdb_selected_path_flags_t flags;
        requiem_string_t *field_name;
        int ret;
        int field_context;

        ret = requiem_string_new(&field_name);
        if ( ret < 0 )
                return ret;

        path = requiemdb_selected_path_get_path(selected);
        flags = requiemdb_selected_path_get_flags(selected);

        if ( flags & (REQUIEMDB_SELECTED_OBJECT_FUNCTION_MIN|
                      REQUIEMDB_SELECTED_OBJECT_FUNCTION_MAX|
                      REQUIEMDB_SELECTED_OBJECT_FUNCTION_AVG|
                      REQUIEMDB_SELECTED_OBJECT_FUNCTION_STD|
                      REQUIEMDB_SELECTED_OBJECT_FUNCTION_COUNT) )
                field_context = FIELD_CONTEXT_FUNCTION;
        else
                field_context = FIELD_CONTEXT_SELECT;

        ret = classic_path_resolve(path, field_context, join, field_name);
        if ( ret < 0 )
                goto error;

        ret = classic_sql_select_add_field(select, requiem_string_get_string(field_name), flags);

 error:
        requiem_string_destroy(field_name);

        return ret;
}



int classic_path_resolve_selection(requiemdb_sql_t *sql,
                                   requiemdb_path_selection_t *selection,
                                   classic_sql_join_t *join, classic_sql_select_t *select)
{
        requiemdb_selected_path_t *selected = NULL;
        int ret;

        while ( (selected = requiemdb_path_selection_get_next(selection, selected)) ) {
                ret = classic_path_resolve_selected(sql, selected, join, select);
                if ( ret < 0 )
                        return ret;
        }

        return 0;
}



static int classic_path_resolve_criterion(requiemdb_sql_t *sql,
                                          idmef_criterion_t *criterion,
                                          classic_sql_join_t *join, requiem_string_t *output)
{
        requiem_string_t *field_name;
        int ret;

        ret = requiem_string_new(&field_name);
        if ( ret < 0 )
                return ret;

        ret = classic_path_resolve(idmef_criterion_get_path(criterion), FIELD_CONTEXT_WHERE, join, field_name);
        if ( ret < 0 )
                goto error;

        ret = requiemdb_sql_build_criterion_string(sql, output,
                                                   requiem_string_get_string(field_name),
                                                   idmef_criterion_get_operator(criterion),
                                                   idmef_criterion_get_value(criterion));

 error:
        requiem_string_destroy(field_name);

        return ret;
}



int classic_path_resolve_criteria(requiemdb_sql_t *sql,
                                  idmef_criteria_t *criteria,
                                  classic_sql_join_t *join, requiem_string_t *output)
{
        int ret;
        idmef_criteria_t *or, *and;

        or = idmef_criteria_get_or(criteria);
        and = idmef_criteria_get_and(criteria);

        if ( or ) {
                ret = requiem_string_cat(output, "((");
                if ( ret < 0 )
                        return ret;
        }

        ret = classic_path_resolve_criterion(sql, idmef_criteria_get_criterion(criteria), join, output);
        if ( ret < 0 )
                return ret;

        if ( and ) {
                ret = requiem_string_cat(output, " AND ");
                if ( ret < 0 )
                        return ret;

                ret = classic_path_resolve_criteria(sql, and, join, output);
                if ( ret < 0 )
                        return ret;
        }

        if ( or ) {
                ret = requiem_string_cat(output, ") OR (");
                if ( ret < 0 )
                        return ret;

                ret = classic_path_resolve_criteria(sql, or, join, output);
                if ( ret < 0 )
                        return ret;

                ret = requiem_string_cat(output, "))");
                if ( ret < 0 )
                        return ret;
        }

        return 0;

}
