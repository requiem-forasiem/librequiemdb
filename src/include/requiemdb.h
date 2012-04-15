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

#ifndef _LIBREQUIEMDB_H
#define _LIBREQUIEMDB_H

#include <librequiem/requiem-inttypes.h>
#include <librequiem/idmef.h>

#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb-error.h"
#include "requiemdb-path-selection.h"

#ifdef __cplusplus
 extern "C" {
#endif


typedef struct requiemdb requiemdb_t;

typedef struct requiemdb_result_idents requiemdb_result_idents_t;
typedef struct requiemdb_result_values requiemdb_result_values_t;

typedef enum {
        REQUIEMDB_RESULT_IDENTS_ORDER_BY_NONE = 0,
        REQUIEMDB_RESULT_IDENTS_ORDER_BY_CREATE_TIME_DESC = 1,
        REQUIEMDB_RESULT_IDENTS_ORDER_BY_CREATE_TIME_ASC = 2
} requiemdb_result_idents_order_t;


#define REQUIEMDB_ERRBUF_SIZE 512

int requiemdb_init(void);
void requiemdb_deinit(void);

int requiemdb_new(requiemdb_t **db, requiemdb_sql_t *sql, const char *format_name, char *errbuf, size_t size);

void requiemdb_destroy(requiemdb_t *db);

const char *requiemdb_get_format_name(requiemdb_t *db);
const char *requiemdb_get_format_version(requiemdb_t *db);

int requiemdb_set_format(requiemdb_t *db, const char *format_name);

requiemdb_sql_t *requiemdb_get_sql(requiemdb_t *db);

void requiemdb_result_idents_destroy(requiemdb_result_idents_t *result);
int requiemdb_result_idents_get_next(requiemdb_result_idents_t *result, uint64_t *ident);

void requiemdb_result_values_destroy(requiemdb_result_values_t *result);
int requiemdb_result_values_get_next(requiemdb_result_values_t *result, idmef_value_t ***values);

char *requiemdb_get_error(requiemdb_t *db, requiemdb_error_t error, char *errbuf, size_t size);

int requiemdb_insert_message(requiemdb_t *db, idmef_message_t *message);

int requiemdb_get_alert_idents(requiemdb_t *db, idmef_criteria_t *criteria,
                               int limit, int offset,
                               requiemdb_result_idents_order_t order,
                               requiemdb_result_idents_t **result);
int requiemdb_get_heartbeat_idents(requiemdb_t *db, idmef_criteria_t *criteria,
                                   int limit, int offset,
                                   requiemdb_result_idents_order_t order,
                                   requiemdb_result_idents_t **result);

int requiemdb_get_alert(requiemdb_t *db, uint64_t ident, idmef_message_t **message);
int requiemdb_get_heartbeat(requiemdb_t *db, uint64_t ident, idmef_message_t **message);

int requiemdb_delete_alert(requiemdb_t *db, uint64_t ident);

ssize_t requiemdb_delete_alert_from_list(requiemdb_t *db, uint64_t *idents, size_t size);

ssize_t requiemdb_delete_alert_from_result_idents(requiemdb_t *db, requiemdb_result_idents_t *result);

int requiemdb_delete_heartbeat(requiemdb_t *db, uint64_t ident);

ssize_t requiemdb_delete_heartbeat_from_list(requiemdb_t *db, uint64_t *idents, size_t size);

ssize_t requiemdb_delete_heartbeat_from_result_idents(requiemdb_t *db, requiemdb_result_idents_t *result);

int requiemdb_get_values(requiemdb_t *db, requiemdb_path_selection_t *path_selection,
                         idmef_criteria_t *criteria, requiem_bool_t distinct, int limit, int offset,
                         requiemdb_result_values_t **result);


int requiemdb_transaction_start(requiemdb_t *db);


int requiemdb_transaction_end(requiemdb_t *db);


int requiemdb_transaction_abort(requiemdb_t *db);


#ifdef __cplusplus
  }
#endif

#endif /* _LIBREQUIEMDB_H */
