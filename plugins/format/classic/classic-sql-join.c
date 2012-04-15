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
#include "requiemdb-error.h"

#include "classic-sql-join.h"


struct classic_sql_joined_table {
        requiem_list_t list;
        const idmef_path_t *path;
        char *table_name;
        char aliased_table_name[16];
        char parent_type;
        requiem_string_t *index_constraints;
};


struct classic_sql_join {
        idmef_class_id_t top_class;
        requiem_list_t tables;
        unsigned int next_id;
};



int classic_sql_join_new(classic_sql_join_t **join)
{
        *join = calloc(1, sizeof (**join));
        if ( ! *join )
                return requiem_error_from_errno(errno);

        requiem_list_init(&(*join)->tables);

        return 0;
}



void classic_sql_join_destroy(classic_sql_join_t *join)
{
        requiem_list_t *tmp, *next;
        classic_sql_joined_table_t *table;

        requiem_list_for_each_safe(&join->tables, tmp, next) {
                table = requiem_list_entry(tmp, classic_sql_joined_table_t, list);
                free(table->table_name);
                requiem_string_destroy(table->index_constraints);
                requiem_list_del(&table->list);
                free(table);
        }

        free(join);
}



void classic_sql_join_set_top_class(classic_sql_join_t *join, idmef_class_id_t top_class)
{
        join->top_class = top_class;
}


classic_sql_joined_table_t *classic_sql_join_lookup_table(const classic_sql_join_t *join, const idmef_path_t *path)
{
        requiem_list_t *tmp;
        classic_sql_joined_table_t *table;
        unsigned int depth;
        int last_element_is_listed;
        int ret;

        depth = idmef_path_get_depth(path);
        last_element_is_listed = ((ret = idmef_path_get_index(path, depth - 1)) > -1 ||
                                  requiem_error_get_code(ret) != REQUIEM_ERROR_IDMEF_PATH_INDEX_FORBIDDEN);

        requiem_list_for_each(&join->tables, tmp) {
                table = requiem_list_entry(tmp, classic_sql_joined_table_t, list);
                if ( depth == idmef_path_get_depth(table->path) ) {
                        if ( last_element_is_listed || (idmef_path_get_value_type(path, -1) == IDMEF_VALUE_TYPE_TIME && idmef_path_get_class(path, depth - 2) != IDMEF_CLASS_ID_FILE) )
                                ret = idmef_path_compare(path, table->path);
                        else
                                ret = idmef_path_ncompare(path, table->path, depth - 1);


                        if ( ret == 0 )
                                return table;
                }
        }

        return NULL;
}



static char resolve_file_parent_type(const idmef_path_t *path)
{
        if ( idmef_path_get_class(path, 3) == IDMEF_CLASS_ID_FILE_ACCESS &&
             idmef_path_get_class(path, 4) == IDMEF_CLASS_ID_USER_ID )
                return 'F';

        return 0;
}



static char resolve_target_parent_type(const idmef_path_t *path)
{
        if ( idmef_path_get_depth(path) == 3 )
                return 0;

        if ( idmef_path_get_class(path, 2) == IDMEF_CLASS_ID_FILE )
                return resolve_file_parent_type(path);

        return 'T';
}



static char resolve_source_parent_type(const idmef_path_t *path)
{
        return (idmef_path_get_depth(path) > 3) ? 'S' : 0;
}



static char resolve_tool_alert_parent_type(const idmef_path_t *path)
{
        if ( idmef_path_get_class(path, 2) == IDMEF_CLASS_ID_ALERTIDENT )
                return 'T';

        return 0;
}



static char resolve_correlation_alert_parent_type(const idmef_path_t *path)
{
        if ( idmef_path_get_class(path, 2) == IDMEF_CLASS_ID_ALERTIDENT )
                return 'C';

        return 0;
}



static char resolve_alert_parent_type(const idmef_path_t *path)
{
        switch ( idmef_path_get_class(path, 1) ) {
        case IDMEF_CLASS_ID_CLASSIFICATION:
        case IDMEF_CLASS_ID_ASSESSMENT:
        case IDMEF_CLASS_ID_OVERFLOW_ALERT:
                return 0;

        case IDMEF_CLASS_ID_TOOL_ALERT:
                return resolve_tool_alert_parent_type(path);

        case IDMEF_CLASS_ID_CORRELATION_ALERT:
                return resolve_correlation_alert_parent_type(path);

        case IDMEF_CLASS_ID_SOURCE:
                return resolve_source_parent_type(path);

        case IDMEF_CLASS_ID_TARGET:
                return resolve_target_parent_type(path);

        default:
                /* nop */;
        }

        if ( strcmp(idmef_path_get_name(path, idmef_path_get_depth(path) - 1), "detect_time") == 0 )
                return 0;

        return 'A';
}



static char resolve_parent_type(const idmef_path_t *path)
{
        if ( idmef_path_get_class(path, 0) == IDMEF_CLASS_ID_HEARTBEAT )
                return 'H';

        return resolve_alert_parent_type(path);
}



static int add_index_constraint(classic_sql_joined_table_t *table, int parent_level, int index)
{
        int ret;
        const char *operator;

        if ( ! requiem_string_is_empty(table->index_constraints) ) {
                ret = requiem_string_cat(table->index_constraints, " AND ");
                if ( ret < 0 )
                        return ret;
        }

        if ( index >= -1 )
                operator = "=";
        else {
                /* index is REQUIEM_ERROR_IDMEF_PATH_INDEX_UNDEFINED */
                index = -1;
                operator = "!=";
        }

        if ( parent_level == -1 )
                ret = requiem_string_sprintf(table->index_constraints, "%s._index %s %d",
                                             table->aliased_table_name, operator, index);
        else
                ret = requiem_string_sprintf(table->index_constraints, "%s._parent%d_index %s %d",
                                             table->aliased_table_name, parent_level, operator, index);

        return ret;
}



static int resolve_indexes(classic_sql_joined_table_t *table)
{
        unsigned int depth;
        unsigned int max_depth;
        unsigned int parent_level;
        int index, index1, index2;
        int ret = 0;

        max_depth = idmef_path_get_depth(table->path);
        if ( max_depth < 2 )
                return requiemdb_error(REQUIEMDB_ERROR_QUERY);

        parent_level = 0;

        for ( depth = 1; depth < max_depth - 2; depth++ ) {
                index = idmef_path_get_index(table->path, depth);
                if ( requiem_error_get_code(index) == REQUIEM_ERROR_IDMEF_PATH_INDEX_FORBIDDEN )
                        continue;

                ret = add_index_constraint(table, parent_level, index);
                if ( ret < 0 )
                        return ret;

                parent_level++;
        }

        index1 = idmef_path_get_index(table->path, max_depth - 1);
        index2 = idmef_path_get_index(table->path, max_depth - 2);

        if ( requiem_error_get_code(index = index1) != REQUIEM_ERROR_IDMEF_PATH_INDEX_FORBIDDEN ||
             requiem_error_get_code(index = index2) != REQUIEM_ERROR_IDMEF_PATH_INDEX_FORBIDDEN )
                ret = add_index_constraint(table, -1, index);

        return ret;
}



int classic_sql_join_new_table(classic_sql_join_t *join, classic_sql_joined_table_t **table,
                               const idmef_path_t *path, char *table_name)
{
        int ret;
        idmef_class_id_t top_class;

        top_class = idmef_path_get_class(path, 0);

        if ( join->top_class ) {
                if ( top_class != join->top_class )
                        return -1; /* conflicting top tables */

        } else {
                join->top_class = top_class;
        }

        *table = calloc(1, sizeof (**table));
        if ( ! *table )
                return requiem_error_from_errno(errno);

        ret = requiem_string_new(&(*table)->index_constraints);
        if ( ret < 0 ) {
                free(*table);
                return ret;
        }

        (*table)->path = path;
        (*table)->table_name = table_name;
        sprintf((*table)->aliased_table_name, "t%d", join->next_id++);

        (*table)->parent_type = resolve_parent_type((*table)->path);

        ret = resolve_indexes(*table);
        if ( ret < 0 ) {
                requiem_string_destroy((*table)->index_constraints);
                free((*table)->table_name);
                free(*table);
                return ret;
        }

        requiem_list_add_tail(&join->tables, &(*table)->list);

        return 0;
}



const char *classic_sql_joined_table_get_name(classic_sql_joined_table_t *table)
{
        return table->aliased_table_name;
}



static int classic_joined_table_to_string(classic_sql_joined_table_t *table, requiem_string_t *output)
{
        int ret;

        ret = requiem_string_sprintf(output, " LEFT JOIN %s AS %s ON (", table->table_name, table->aliased_table_name);
        if ( ret < 0 )
                return ret;

        if ( table->parent_type ) {
                ret = requiem_string_sprintf(output, "%s._parent_type='%c' AND ",
                                             table->aliased_table_name, table->parent_type);
                if ( ret < 0 )
                        return ret;
        }

        ret = requiem_string_sprintf(output, "%s._message_ident=top_table._ident", table->aliased_table_name);
        if ( ret < 0 )
                return ret;

        if ( ! requiem_string_is_empty(table->index_constraints) ) {
                ret = requiem_string_sprintf(output, " AND %s", requiem_string_get_string(table->index_constraints));
                if ( ret < 0 )
                        return ret;
        }

        return requiem_string_cat(output, ")");
}



int classic_sql_join_to_string(classic_sql_join_t *join, requiem_string_t *output)
{
        requiem_list_t *tmp;
        classic_sql_joined_table_t *table;
        int ret;

        ret = requiem_string_sprintf(output, "%s AS top_table",
                                     (join->top_class == IDMEF_CLASS_ID_ALERT) ? "Requiem_Alert" : "Requiem_Heartbeat");
        if ( ret < 0 )
                return ret;

        requiem_list_for_each(&join->tables, tmp) {
                table = requiem_list_entry(tmp, classic_sql_joined_table_t, list);
                ret = classic_joined_table_to_string(table, output);
                if ( ret < 0 )
                        return ret;
        }

        return 0;
}
