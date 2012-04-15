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

#ifndef _LIBREQUIEMDB_SQL_H
#define _LIBREQUIEMDB_SQL_H

#include <librequiem/requiem-string.h>
#include <librequiem/idmef-criteria.h>
#include <librequiem/idmef-time.h>
#include "requiemdb-sql-settings.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(Spec) /* empty */
# endif
/* The __-protected variants of `format' and `printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif

#define REQUIEMDB_SQL_TIMESTAMP_STRING_SIZE 128

typedef enum {
	REQUIEMDB_SQL_TIME_CONSTRAINT_YEAR,
	REQUIEMDB_SQL_TIME_CONSTRAINT_MONTH,
	REQUIEMDB_SQL_TIME_CONSTRAINT_YDAY,
	REQUIEMDB_SQL_TIME_CONSTRAINT_MDAY,
	REQUIEMDB_SQL_TIME_CONSTRAINT_WDAY,
	REQUIEMDB_SQL_TIME_CONSTRAINT_HOUR,
	REQUIEMDB_SQL_TIME_CONSTRAINT_MIN,
	REQUIEMDB_SQL_TIME_CONSTRAINT_SEC
} requiemdb_sql_time_constraint_type_t;

		
typedef struct requiemdb_sql requiemdb_sql_t;

typedef struct requiemdb_sql_table requiemdb_sql_table_t;
typedef struct requiemdb_sql_row requiemdb_sql_row_t;
typedef struct requiemdb_sql_field requiemdb_sql_field_t;

int requiemdb_sql_new(requiemdb_sql_t **newdb, const char *type, requiemdb_sql_settings_t *settings);

void requiemdb_sql_destroy(requiemdb_sql_t *sql);

int requiemdb_sql_enable_query_logging(requiemdb_sql_t *sql, const char *filename);
void requiemdb_sql_disable_query_logging(requiemdb_sql_t *sql);

int requiemdb_sql_query(requiemdb_sql_t *sql, const char *query, requiemdb_sql_table_t **table);

int requiemdb_sql_query_sprintf(requiemdb_sql_t *sql, requiemdb_sql_table_t **table, const char *format, ...)
                                __attribute__ ((__format__ (__printf__, 3, 4)));

int requiemdb_sql_insert(requiemdb_sql_t *sql, const char *table, const char *fields, const char *format, ...)
                         __attribute__ ((__format__ (__printf__, 4, 5)));

int requiemdb_sql_build_limit_offset_string(requiemdb_sql_t *sql, int limit, int offset, requiem_string_t *output);

int requiemdb_sql_transaction_start(requiemdb_sql_t *sql);
int requiemdb_sql_transaction_end(requiemdb_sql_t *sql);
int requiemdb_sql_transaction_abort(requiemdb_sql_t *sql);

int requiemdb_sql_escape_fast(requiemdb_sql_t *sql, const char *input, size_t input_size, char **output);
int requiemdb_sql_escape(requiemdb_sql_t *sql, const char *input, char **output);
int requiemdb_sql_escape_binary(requiemdb_sql_t *sql, const unsigned char *input, size_t input_size, char **output);
int requiemdb_sql_unescape_binary(requiemdb_sql_t *sql, const char *input, size_t input_size,
				  unsigned char **output, size_t *output_size);

void requiemdb_sql_table_destroy(requiemdb_sql_table_t *table);
const char *requiemdb_sql_table_get_column_name(requiemdb_sql_table_t *table, unsigned int column_num);
int requiemdb_sql_table_get_column_num(requiemdb_sql_table_t *table, const char *column_name);
unsigned int requiemdb_sql_table_get_column_count(requiemdb_sql_table_t *table);
unsigned int requiemdb_sql_table_get_row_count(requiemdb_sql_table_t *table);
int requiemdb_sql_table_fetch_row(requiemdb_sql_table_t *table, requiemdb_sql_row_t **row);

int requiemdb_sql_row_fetch_field(requiemdb_sql_row_t *row, unsigned int column_num, requiemdb_sql_field_t **field);
int requiemdb_sql_row_fetch_field_by_name(requiemdb_sql_row_t *row, const char *column_name, requiemdb_sql_field_t **field);

const char *requiemdb_sql_field_get_value(requiemdb_sql_field_t *field);
size_t requiemdb_sql_field_get_len(requiemdb_sql_field_t *field);
int requiemdb_sql_field_to_int8(requiemdb_sql_field_t *field, int8_t *value);
int requiemdb_sql_field_to_uint8(requiemdb_sql_field_t *field, uint8_t *value);
int requiemdb_sql_field_to_int16(requiemdb_sql_field_t *field, int16_t *value);
int requiemdb_sql_field_to_uint16(requiemdb_sql_field_t *field, uint16_t *value);
int requiemdb_sql_field_to_int32(requiemdb_sql_field_t *field, int32_t *value);
int requiemdb_sql_field_to_uint32(requiemdb_sql_field_t *field, uint32_t *value);
int requiemdb_sql_field_to_int64(requiemdb_sql_field_t *field, int64_t *value);
int requiemdb_sql_field_to_uint64(requiemdb_sql_field_t *field, uint64_t *value);
int requiemdb_sql_field_to_float(requiemdb_sql_field_t *field, float *value);
int requiemdb_sql_field_to_double(requiemdb_sql_field_t *field, double *value);
int requiemdb_sql_field_to_string(requiemdb_sql_field_t *field, requiem_string_t *output);

int requiemdb_sql_build_criterion_string(requiemdb_sql_t *sql,
					 requiem_string_t *output,
					 const char *field,
					 idmef_criterion_operator_t idmef_operator, idmef_criterion_value_t *value);

int requiemdb_sql_time_from_timestamp(idmef_time_t *time, const char *time_buf, int32_t gmtoff, uint32_t usec);
int requiemdb_sql_time_to_timestamp(requiemdb_sql_t *sql,
                                    const idmef_time_t *time,
				    char *time_buf, size_t time_buf_size,
				    char *gmtoff_buf, size_t gmtoff_buf_size,
				    char *usec_buf, size_t usec_buf_size);

/*
 * Deprecated, use requiemdb_strerror()
 */
const char *requiemdb_sql_get_plugin_error(requiemdb_sql_t *sql);
         
#ifdef __cplusplus
  }
#endif

#endif /* _LIBREQUIEMDB_SQL_H */
