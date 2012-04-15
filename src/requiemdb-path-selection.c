/*****
*
* Copyright (C) 2003-2005,2006,2007 PreludeIDS Technologies. All Rights Reserved.
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

#include <librequiem/idmef.h>
#include <librequiem/requiem-list.h>
#include <librequiem/requiem-log.h>

#include "requiemdb-error.h"
#include "requiemdb-path-selection.h"

struct requiemdb_selected_path {
        requiem_list_t list;
        idmef_path_t *path;
        requiemdb_selected_path_flags_t flags;
};

struct requiemdb_path_selection {
        requiem_list_t list;
        size_t count;
};



int requiemdb_selected_path_new(requiemdb_selected_path_t **selected_path,
                                idmef_path_t *path, int flags)
{
        *selected_path = calloc(1, sizeof (**selected_path));
        if ( ! *selected_path )
                return requiemdb_error_from_errno(errno);

        (*selected_path)->path = path;
        (*selected_path)->flags = flags;

        return 0;
}



static int parse_filter(const char *str, size_t len)
{
        unsigned int i;
        struct {
                const char *name;
                int flag;
        } filter_table[] = {
                { "group_by",        REQUIEMDB_SELECTED_OBJECT_GROUP_BY        },
                { "order_desc",        REQUIEMDB_SELECTED_OBJECT_ORDER_DESC        },
                { "order_asc",        REQUIEMDB_SELECTED_OBJECT_ORDER_ASC        }
        };

        for ( i = 0; i < sizeof(filter_table) / sizeof(*filter_table); i++ ) {
                if ( strncmp(str, filter_table[i].name, len) == 0 )
                        return filter_table[i].flag;
        }

        return requiemdb_error_verbose(REQUIEMDB_ERROR_INVALID_SELECTED_OBJECT_STRING, "Invalid path filter string '%s'", str);
}



static int parse_filters(const char *str)
{
        int flags = 0, ret;
        const char *start, *end;

        start = str;

        while ( (end = strchr(start, ',')) ) {
                ret = parse_filter(start, end - start);
                if ( ret < 0 )
                        return ret;

                flags |= ret;

                start = end + 1;
        }

        ret = parse_filter(start, strlen(start));

        return (ret < 0) ? ret : (flags | ret);
}



static int parse_function(const char *str)
{
        unsigned int i;
        struct {
                const char *name;
                size_t len;
                int flag;
        } function_table[] = {
                { "min(",   4, REQUIEMDB_SELECTED_OBJECT_FUNCTION_MIN        },
                { "max(",   4, REQUIEMDB_SELECTED_OBJECT_FUNCTION_MAX        },
                { "avg(",   4, REQUIEMDB_SELECTED_OBJECT_FUNCTION_AVG        },
                { "std(",   4, REQUIEMDB_SELECTED_OBJECT_FUNCTION_STD        },
                { "count(", 6, REQUIEMDB_SELECTED_OBJECT_FUNCTION_COUNT }
        };

        for ( i = 0; i < sizeof(function_table) / sizeof(*function_table); i++ ) {
                if ( strncmp(str, function_table[i].name, function_table[i].len) == 0 )
                        return function_table[i].flag;
        }

        return 0;
}


static int parse_path(const char *str, size_t len, idmef_path_t **path)
{
        char buf[len + 1];

        if ( len + 1 < len )
                return -1;

        memcpy(buf, str, len);
        buf[len] = 0;

        return idmef_path_new_fast(path, buf);
}


int requiemdb_selected_path_new_string(requiemdb_selected_path_t **selected_path, const char *str)
{
        int ret, flags = 0;
        idmef_path_t *path;
        const char *filters, *start, *end;

        filters = strchr(str, '/');
        if ( filters ) {
                ret = parse_filters(filters + 1);
                if ( ret < 0 )
                        return ret;

                flags |= ret;
        }

        ret = parse_function(str);
        if ( ret < 0 )
                return ret;

        if ( ret ) {
                flags |= ret;

                if ( ! (start = strchr(str, '(')) || ! (end = strrchr(str, ')')) )
                        return requiemdb_error(REQUIEMDB_ERROR_INVALID_SELECTED_OBJECT_STRING);

                ret = parse_path(start + 1, end - (start + 1), &path);
        } else {
                if ( filters )
                        ret = parse_path(str, filters - str, &path);
                else
                        ret = idmef_path_new_fast(&path, str);
        }

        if ( ret < 0 )
                return ret;

        ret = requiemdb_selected_path_new(selected_path, path, flags);
        if ( ret < 0 )
                idmef_path_destroy(path);

        return ret;
}



void requiemdb_selected_path_destroy(requiemdb_selected_path_t *selected_path)
{
        idmef_path_destroy(selected_path->path);
        free(selected_path);
}



idmef_path_t *requiemdb_selected_path_get_path(requiemdb_selected_path_t *selected_path)
{
        return selected_path->path;
}



requiemdb_selected_path_flags_t requiemdb_selected_path_get_flags(requiemdb_selected_path_t *selected_path)
{
        return selected_path->flags;
}



int requiemdb_path_selection_new(requiemdb_path_selection_t **path_selection)
{
        *path_selection = calloc(1, sizeof(**path_selection));
        if ( ! *path_selection )
                return requiemdb_error_from_errno(errno);

        (*path_selection)->count = 0;
        requiem_list_init(&(*path_selection)->list);

        return 0;
}



void requiemdb_path_selection_destroy(requiemdb_path_selection_t *path_selection)
{
        requiem_list_t *ptr, *next;
        requiemdb_selected_path_t *selected_path;

        requiem_list_for_each_safe(&path_selection->list, ptr, next) {
                selected_path = requiem_list_entry(ptr, requiemdb_selected_path_t, list);
                requiemdb_selected_path_destroy(selected_path);
        }

        free(path_selection);
}



void requiemdb_path_selection_add(requiemdb_path_selection_t *path_selection,
                                  requiemdb_selected_path_t *selected_path)
{
        path_selection->count++;
        requiem_list_add_tail(&path_selection->list, &selected_path->list);
}



requiemdb_selected_path_t *requiemdb_path_selection_get_next(requiemdb_path_selection_t *path_selection,
                                                             requiemdb_selected_path_t *selected_path)
{
        return requiem_list_get_next(&path_selection->list, selected_path, requiemdb_selected_path_t, list);
}



size_t requiemdb_path_selection_get_count(requiemdb_path_selection_t *path_selection)
{
        return path_selection->count;
}
