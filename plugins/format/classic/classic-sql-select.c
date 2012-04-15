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

#include "classic-sql-select.h"


struct classic_sql_select {
        requiem_string_t *fields;
        unsigned int field_count;
        requiem_string_t *order_by;
        requiem_string_t *group_by;
};



int classic_sql_select_new(classic_sql_select_t **select)
{
        int ret;

        *select = calloc(1, sizeof (**select));
        if ( ! *select )
                return requiem_error_from_errno(errno);

        ret = requiem_string_new(&(*select)->fields);
        if ( ret < 0 ) {
                free(*select);
                return ret;
        }

        ret = requiem_string_new(&(*select)->order_by);
        if ( ret < 0 ) {
                requiem_string_destroy((*select)->fields);
                free(*select);
                return ret;
        }

        ret = requiem_string_new(&(*select)->group_by);
        if ( ret < 0 ) {
                requiem_string_destroy((*select)->fields);
                requiem_string_destroy((*select)->order_by);
                free(*select);
                return ret;
        }

        return 0;
}



void classic_sql_select_destroy(classic_sql_select_t *select)
{
        requiem_string_destroy(select->fields);
        requiem_string_destroy(select->order_by);
        requiem_string_destroy(select->group_by);
        free(select);
}



int classic_sql_select_add_field(classic_sql_select_t *select, const char *field_name,
                                 requiemdb_selected_path_flags_t flags)
{
        static const struct {
                int flag;
                const char *function_name;
        } sql_functions_table[] = {
                { REQUIEMDB_SELECTED_OBJECT_FUNCTION_MIN, "MIN" },
                { REQUIEMDB_SELECTED_OBJECT_FUNCTION_MAX, "MAX" },
                { REQUIEMDB_SELECTED_OBJECT_FUNCTION_AVG, "AVG" },
                { REQUIEMDB_SELECTED_OBJECT_FUNCTION_STD, "STD" },
                { REQUIEMDB_SELECTED_OBJECT_FUNCTION_COUNT, "COUNT" }
        };
        unsigned int i;
        const char *function_name = NULL;
        int ret;

        if ( ! requiem_string_is_empty(select->fields) ) {
                ret = requiem_string_cat(select->fields, ", ");
                if ( ret < 0 )
                        return ret;
        }

        for ( i = 0; i < sizeof (sql_functions_table) / sizeof (sql_functions_table[0]); i++ ) {
                if ( flags & sql_functions_table[i].flag ) {
                        function_name = sql_functions_table[i].function_name;
                        break;
                }
        }

        if ( function_name )
                ret = requiem_string_sprintf(select->fields, "%s(%s)", function_name, field_name);
        else
                ret = requiem_string_cat(select->fields, field_name);

        if ( ret < 0 )
                return ret;

        select->field_count++;

        if ( flags & REQUIEMDB_SELECTED_OBJECT_GROUP_BY ) {
                if ( ! requiem_string_is_empty(select->group_by) ) {
                        ret = requiem_string_cat(select->group_by, ", ");
                        if ( ret < 0 )
                                return ret;
                }

                ret = requiem_string_sprintf(select->group_by, "%d", select->field_count);
                if ( ret < 0 )
                        return ret;
        }

        if ( flags & (REQUIEMDB_SELECTED_OBJECT_ORDER_ASC|REQUIEMDB_SELECTED_OBJECT_ORDER_DESC) ) {
                if ( ! requiem_string_is_empty(select->order_by) ) {
                        ret = requiem_string_cat(select->order_by, ", ");
                        if ( ret < 0 )
                                return ret;
                }

                ret = requiem_string_sprintf(select->order_by, "%d %s", select->field_count,
                                             (flags & REQUIEMDB_SELECTED_OBJECT_ORDER_ASC) ? "ASC" : "DESC");
                if ( ret < 0 )
                        return ret;
        }

        return 0;
}



int classic_sql_select_fields_to_string(classic_sql_select_t *select, requiem_string_t *output)
{
        return requiem_string_ncat(output,
                                   requiem_string_get_string(select->fields), requiem_string_get_len(select->fields));
}



int classic_sql_select_modifiers_to_string(classic_sql_select_t *select, requiem_string_t *output)
{
        int ret;

        if ( ! requiem_string_is_empty(select->group_by) ) {
                ret = requiem_string_sprintf(output, " GROUP BY %s", requiem_string_get_string(select->group_by));
                if ( ret < 0 )
                        return ret;
        }

        if ( ! requiem_string_is_empty(select->order_by) ) {
                ret = requiem_string_sprintf(output, " ORDER BY %s", requiem_string_get_string(select->order_by));
                if ( ret < 0 )
                        return ret;
        }

        return 0;
}
