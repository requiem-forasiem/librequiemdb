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

#ifndef _LIBREQUIEMDB_OBJECT_SELECTION_H
#define _LIBREQUIEMDB_OBJECT_SELECTION_H

#ifdef __cplusplus
 extern "C" {
#endif

typedef enum {
	REQUIEMDB_SELECTED_OBJECT_FUNCTION_MIN = 0x01,
	REQUIEMDB_SELECTED_OBJECT_FUNCTION_MAX = 0x02,
	REQUIEMDB_SELECTED_OBJECT_FUNCTION_AVG = 0x04,
	REQUIEMDB_SELECTED_OBJECT_FUNCTION_STD = 0x08,
	REQUIEMDB_SELECTED_OBJECT_FUNCTION_COUNT = 0x10,

	REQUIEMDB_SELECTED_OBJECT_GROUP_BY = 0x20,

	REQUIEMDB_SELECTED_OBJECT_ORDER_ASC = 0x40,
	REQUIEMDB_SELECTED_OBJECT_ORDER_DESC = 0x80
} requiemdb_selected_path_flags_t;


typedef struct requiemdb_path_selection requiemdb_path_selection_t;
typedef struct requiemdb_selected_path requiemdb_selected_path_t;

int requiemdb_selected_path_new(requiemdb_selected_path_t **selected_path,
				  idmef_path_t *path, int flags);
int requiemdb_selected_path_new_string(requiemdb_selected_path_t **selected_path, const char *str);
void requiemdb_selected_path_destroy(requiemdb_selected_path_t *selected_path);
idmef_path_t *requiemdb_selected_path_get_path(requiemdb_selected_path_t *selected_path);
requiemdb_selected_path_flags_t requiemdb_selected_path_get_flags(requiemdb_selected_path_t *selected_path);

int requiemdb_path_selection_new(requiemdb_path_selection_t **path_selection);
void requiemdb_path_selection_destroy(requiemdb_path_selection_t *path_selection);
void requiemdb_path_selection_add(requiemdb_path_selection_t *path_selection,
				    requiemdb_selected_path_t *selected_path);
requiemdb_selected_path_t *requiemdb_path_selection_get_next(requiemdb_path_selection_t *path_selection,
								 requiemdb_selected_path_t *selected_path);
size_t requiemdb_path_selection_get_count(requiemdb_path_selection_t *path_selection);

#ifdef __cplusplus
  }
#endif

#endif /* ! _LIBREQUIEMDB_OBJECT_SELECTION_H */
