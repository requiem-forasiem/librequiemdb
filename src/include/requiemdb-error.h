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

#ifndef _LIBREQUIEMDB_ERROR_H
#define _LIBREQUIEMDB_ERROR_H

#include <librequiem/requiem-inttypes.h>
#include <librequiem/requiem-error.h>

#ifdef __cplusplus
 extern "C" {
#endif

typedef enum {
        REQUIEMDB_ERROR_NO_ERROR = 0,
        REQUIEMDB_ERROR_GENERIC = 1,
        REQUIEMDB_ERROR_INVALID_SETTINGS_STRING = 2,
        REQUIEMDB_ERROR_CONNECTION = 3,
        REQUIEMDB_ERROR_QUERY = 4,
        REQUIEMDB_ERROR_INVALID_COLUMN_NUM = 5,
        REQUIEMDB_ERROR_INVALID_COLUMN_NAME = 6,
        REQUIEMDB_ERROR_INVALID_VALUE = 7,
        REQUIEMDB_ERROR_INVALID_VALUE_TYPE = 8,
        REQUIEMDB_ERROR_UNKNOWN_SQL_PLUGIN = 9,
        REQUIEMDB_ERROR_UNKNOWN_FORMAT_PLUGIN = 10,
        REQUIEMDB_ERROR_ALREADY_IN_TRANSACTION = 11,
        REQUIEMDB_ERROR_NOT_IN_TRANSACTION = 12,
	REQUIEMDB_ERROR_INVALID_MESSAGE_IDENT = 13,
	REQUIEMDB_ERROR_INVALID_SELECTED_OBJECT_STRING = 14,
	REQUIEMDB_ERROR_INVALID_OBJECT_SELECTION = 15,
	REQUIEMDB_ERROR_SCHEMA_VERSION_TOO_OLD = 16,
	REQUIEMDB_ERROR_SCHEMA_VERSION_TOO_RECENT = 17,
	REQUIEMDB_ERROR_SCHEMA_VERSION_INVALID = 18,
	REQUIEMDB_ERROR_CANNOT_LOAD_SQL_PLUGIN = 19,
	REQUIEMDB_ERROR_CANNOT_LOAD_FORMAT_PLUGIN = 20
	
} requiemdb_error_code_t;

typedef requiem_error_t requiemdb_error_t;


static inline requiemdb_error_t requiemdb_error(requiemdb_error_code_t error)
{
        return (requiemdb_error_t) requiem_error_make(REQUIEM_ERROR_SOURCE_REQUIEMDB, (requiem_error_code_t) error);
}



static inline requiemdb_error_t requiemdb_error_verbose(requiemdb_error_code_t error, const char *fmt, ...)
{
        int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = requiem_error_verbose_make_v(REQUIEM_ERROR_SOURCE_REQUIEMDB, (requiem_error_code_t) error, fmt, ap);
	va_end(ap);

	return ret;
}



static inline requiem_bool_t requiemdb_error_check(requiemdb_error_t error,
						   requiemdb_error_code_t code)
{
	return (requiem_bool_t) (requiem_error_get_source(error) == REQUIEM_ERROR_SOURCE_REQUIEMDB &&
		                 requiem_error_get_code(error) == (requiem_error_code_t) code);
}


static inline requiemdb_error_t requiemdb_error_from_errno(int err)
{
	return requiem_error_from_errno(err);
}


const char *requiemdb_strerror(requiemdb_error_t error);

#ifdef __cplusplus
  }
#endif

#endif /* _LIBREQUIEMDB_ERROR_H */
