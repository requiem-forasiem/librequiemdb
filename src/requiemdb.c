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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <librequiem/requiem.h>


#include "requiemdb-error.h"
#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb-path-selection.h"

#include "requiemdb.h"
#include "requiemdb-plugin-format.h"
#include "requiemdb-plugin-format-prv.h"


#define REQUIEMDB_PLUGIN_SYMBOL "requiemdb_plugin_init"


struct requiemdb {
        char *format_version;
        requiemdb_sql_t *sql;
        requiemdb_plugin_format_t *plugin;
};

struct requiemdb_result_idents {
        requiemdb_t *db;
        void *res;
};

struct requiemdb_result_values {
        requiemdb_t *db;
        requiemdb_path_selection_t *selection;
        void *res;
};




int _requiemdb_sql_transaction_start(requiemdb_sql_t *sql);
int _requiemdb_sql_transaction_end(requiemdb_sql_t *sql);
int _requiemdb_sql_transaction_abort(requiemdb_sql_t *sql);
void _requiemdb_sql_enable_internal_transaction(requiemdb_sql_t *sql);
void _requiemdb_sql_disable_internal_transaction(requiemdb_sql_t *sql);



static int librequiemdb_refcount = 0;
REQUIEM_LIST(_sql_plugin_list);
static REQUIEM_LIST(plugin_format_list);



int requiemdb_init(void)
{
        int ret;

        if ( librequiemdb_refcount++ > 0 )
                return 0;

        ret = requiem_init(NULL, NULL);
        if ( ret < 0 )
                return ret;

        ret = access(FORMAT_PLUGIN_DIR, F_OK);
        if ( ret < 0 )
                return requiemdb_error_verbose(REQUIEMDB_ERROR_CANNOT_LOAD_FORMAT_PLUGIN,
                                               "could not access format plugin directory '%s'", FORMAT_PLUGIN_DIR);

        ret = requiem_plugin_load_from_dir(&plugin_format_list, FORMAT_PLUGIN_DIR,
                                           REQUIEMDB_PLUGIN_SYMBOL, NULL, NULL, NULL);
        if ( ret < 0 )
                return ret;

        ret = access(SQL_PLUGIN_DIR, F_OK);
        if ( ret < 0 )
                return requiemdb_error_verbose(REQUIEMDB_ERROR_CANNOT_LOAD_SQL_PLUGIN,
                                               "could not access sql plugin directory '%s'", SQL_PLUGIN_DIR);

        ret = requiem_plugin_load_from_dir(&_sql_plugin_list, SQL_PLUGIN_DIR,
                                           REQUIEMDB_PLUGIN_SYMBOL, NULL, NULL, NULL);
        if ( ret < 0 )
                return ret;

        return 0;
}



void requiemdb_deinit(void)
{
        requiem_list_t *iter;
        requiem_plugin_generic_t *pl;

        if ( --librequiemdb_refcount > 0 )
                return;

        iter = NULL;
        while ( (pl = requiem_plugin_get_next(&_sql_plugin_list, &iter)) ) {
                requiem_plugin_unload(pl);
                free(pl);
        }

        iter = NULL;
        while ( (pl = requiem_plugin_get_next(&plugin_format_list, &iter)) ) {
                requiem_plugin_unload(pl);
                free(pl);
        }

        requiem_deinit();
}



static int requiemdb_autodetect_format(requiemdb_t *db)
{
        requiemdb_sql_table_t *table;
        requiemdb_sql_row_t *row;
        requiemdb_sql_field_t *format_name;
        requiemdb_sql_field_t *format_version;
        int ret;

        ret = requiemdb_sql_query(db->sql, "SELECT name, version from _format", &table);
        if ( ret <= 0 )
                return (ret < 0) ? ret : -1;

        ret = requiemdb_sql_table_fetch_row(table, &row);
        if ( ret < 0 )
                goto error;

        ret = requiemdb_sql_row_fetch_field(row, 0, &format_name);
        if ( ret < 0 )
                goto error;

        ret = requiemdb_set_format(db, requiemdb_sql_field_get_value(format_name));
        if ( ret < 0 )
                goto error;

        ret = requiemdb_sql_row_fetch_field(row, 1, &format_version);
        if ( ret < 0 )
                goto error;

        ret = db->plugin->check_schema_version(requiemdb_sql_field_get_value(format_version));
        if ( ret < 0 )
                goto error;

        db->format_version = strdup(requiemdb_sql_field_get_value(format_version));
        if ( ! db->format_version )
                ret = requiem_error_from_errno(errno);

 error:
        requiemdb_sql_table_destroy(table);

        return ret;

}



/**
 * requiemdb_new:
 * @db: Pointer to a db object to initialize.
 * @sql: Pointer to a sql object.
 * @format_name: Format name of the underlying database, if NULL the format will be automatically detected
 * @errbuf: Buffer that will be set to an error message if an error occur.
 * @size: size of @errbuf.
 *
 * This function initialize the @db object and detect the format of the underlying database if no format name
 * is given.
 *
 * Returns: 0 on success or a negative value if an error occur.
 */
int requiemdb_new(requiemdb_t **db, requiemdb_sql_t *sql, const char *format_name, char *errbuf, size_t size)
{
        int ret;

        *db = calloc(1, sizeof (**db));
        if ( ! *db ) {
                ret = requiemdb_error_from_errno(errno);
                snprintf(errbuf, size, "%s", requiemdb_strerror(ret));
                return ret;
        }

        (*db)->sql = sql;

        if ( format_name )
                ret = requiemdb_set_format(*db, format_name);
        else
                ret = requiemdb_autodetect_format(*db);

        if ( ret < 0 ) {
                if ( errbuf )
                        requiemdb_get_error(*db, ret, errbuf, size);

                if ( (*db)->format_version )
                        free((*db)->format_version);

                free(*db);
        }

        return ret;
}



/**
 * requiemdb_destroy:
 * @db: Pointer to a db object.
 *
 * Destroy @db object and the underlying @sql object given as argument to requiemdb_new.
 */
void requiemdb_destroy(requiemdb_t *db)
{
        requiemdb_sql_destroy(db->sql);
        free(db->format_version);
        free(db);
}



/**
 * requiemdb_get_format_name:
 * @db: Pointer to a db object.
 *
 * Returns: the format name currently used by the @db object.
 */
const char *requiemdb_get_format_name(requiemdb_t *db)
{
        return requiem_plugin_get_name(db->plugin);
}



/**
 * requiemdb_get_format_version:
 * @db: Pointer to a db object.
 *
 * Returns: the format version currently used by the @db object.
 */
const char *requiemdb_get_format_version(requiemdb_t *db)
{
        return db->format_version;
}



/**
 * requiemdb_set_format:
 * @db: Pointer to a db object.
 * @format_name: New format to use.
 *
 * Change the current format plugin.
 *
 * Returns: 0 on success or negative value if an error occur.
 */
int requiemdb_set_format(requiemdb_t *db, const char *format_name)
{
        db->plugin = (requiemdb_plugin_format_t *) requiem_plugin_search_by_name(&plugin_format_list, format_name);
        if ( ! db->plugin )
                return requiemdb_error_verbose(REQUIEMDB_ERROR_CANNOT_LOAD_FORMAT_PLUGIN,
                                               "could not find format plugin '%s'", format_name);

        return 0;
}



/**
 * requiemdb_get_sql:
 * @db: Pointer to a db object.
 *
 * Returns: a pointer to the underlying sql object.
 */
requiemdb_sql_t *requiemdb_get_sql(requiemdb_t *db)
{
        return db->sql;
}



/**
 * requiemdb_get_error:
 * @db: Pointer to a db object.
 * @error: Error code to build the error string from.
 * @errbuf: Buffer where the error message will be stored,
 * @size: size of this buffer must be REQUIEMDB_ERRBUF_SIZE.
 *
 * Build an error message from the error code given as argument and from
 * the sql plugin error string (if any) if the error code is db related.
 *
 * Returns: a pointer to the error string or NULL if an error occured.
 */
char *requiemdb_get_error(requiemdb_t *db, requiemdb_error_t error, char *errbuf, size_t size)
{
        int ret;
        requiemdb_error_t tmp;

        tmp = requiemdb_error(requiem_error_get_code(error));

        ret = snprintf(errbuf, size, "%s: %s", requiemdb_strerror(tmp), requiemdb_strerror(error));
        if ( ret < 0 || (size_t) ret >= size )
                return NULL;

        return errbuf;
}



/**
 * requiemdb_insert_message:
 * @db: Pointer to a db object.
 * @message: Pointer to an IDMEF message.
 *
 * Insert an IDMEF message into the database.
 *
 * Returns: 0 on success, or a negative value if an error occur.
 */
int requiemdb_insert_message(requiemdb_t *db, idmef_message_t *message)
{
        return db->plugin->insert_message(db->sql, message);
}



/**
 * requiemdb_result_idents_destroy:
 * @result: Pointer to an idents result object.
 *
 * Destroy the @result object.
 */
void requiemdb_result_idents_destroy(requiemdb_result_idents_t *result)
{
        result->db->plugin->destroy_message_idents_resource(result->res);
        free(result);
}


/**
 * requiemdb_result_idents_get_next:
 * @result: Pointer to an idents result object.
 * @ident: Pointer to an ident where the next ident will be stored.
 *
 * Retrieve the next ident from the idents result object.
 *
 * Returns: 1 if an ident is available, 0 if there is no more idents available or
 * a negative value if an error occur.
 */
int requiemdb_result_idents_get_next(requiemdb_result_idents_t *result, uint64_t *ident)
{
        return result->db->plugin->get_next_message_ident(result->res, ident);
}


/**
 * requiemdb_result_values_destroy:
 * @result: Pointer to a result values object.
 *
 * Destroy the @result object.
 */
void requiemdb_result_values_destroy(requiemdb_result_values_t *result)
{
        result->db->plugin->destroy_values_resource(result->res);
        free(result);
}



/**
 * requiemdb_result_values_get_next:
 * @result: Pointer to a values result object.
 * @values: Pointer to a values array where the next row of values will be stored.
 *
 * Retrieve the next values row from the values result object.
 *
 * Returns: the number of returned values, 0 if there are no values or a negative value if an
 * error occur.
 */
int requiemdb_result_values_get_next(requiemdb_result_values_t *result, idmef_value_t ***values)
{
        return result->db->plugin->get_next_values(result->res, result->selection, values);
}



static int
requiemdb_get_message_idents(requiemdb_t *db,
                             idmef_criteria_t *criteria,
                             int (*get_idents)(requiemdb_sql_t *sql, idmef_criteria_t *criteria,
                                               int limit, int offset,
                                               requiemdb_result_idents_order_t order,
                                               void **res),
                             int limit, int offset,
                             requiemdb_result_idents_order_t order,
                             requiemdb_result_idents_t **result)
{
        int ret;

        *result = calloc(1, sizeof(**result));
        if ( ! *result )
                return requiemdb_error_from_errno(errno);

        (*result)->db = db;

        ret = get_idents(db->sql, criteria, limit, offset, order, &(*result)->res);
        if ( ret <= 0 )
                free(*result);

        return ret;
}



/**
 * requiemdb_get_alert_idents:
 * @db: Pointer to a db object.
 * @criteria: Pointer to an idmef criteria.
 * @limit: Limit of results or -1 if no limit.
 * @offset: Offset in results or -1 if no offset.
 * @order: Result order.
 * @result: Idents result.
 *
 * Returns: the number of result or a negative value if an error occured.
 */
int requiemdb_get_alert_idents(requiemdb_t *db,
                               idmef_criteria_t *criteria, int limit, int offset,
                               requiemdb_result_idents_order_t order,
                               requiemdb_result_idents_t **result)
{
        return requiemdb_get_message_idents(db, criteria, db->plugin->get_alert_idents, limit, offset, order, result);
}



/**
 * requiemdb_get_heartbeat_idents:
 * @db: Pointer to a db object.
 * @criteria: Pointer to an idmef criteria.
 * @limit: Limit of results or -1 if no limit.
 * @offset: Offset in results or -1 if no offset.
 * @order: Result order.
 * @result: Idents result.
 *
 * Returns: the number of result or a negative value if an error occured.
 */
int requiemdb_get_heartbeat_idents(requiemdb_t *db,
                                   idmef_criteria_t *criteria, int limit, int offset,
                                   requiemdb_result_idents_order_t order,
                                   requiemdb_result_idents_t **result)
{
        return requiemdb_get_message_idents(db, criteria, db->plugin->get_heartbeat_idents, limit, offset, order, result);
}



/**
 * requiemdb_get_alert:
 * @db: Pointer to a db object.
 * @ident: Internal database ident of the alert.
 * @message: Pointer to an idmef message object where the retrieved message will be stored.
 *
 * Returns: 0 on success or a negative value if an error occur.
 */
int requiemdb_get_alert(requiemdb_t *db, uint64_t ident, idmef_message_t **message)
{
        return db->plugin->get_alert(db->sql, ident, message);
}



/**
 * requiemdb_get_heartbeat:
 * @db: Pointer to a db object.
 * @ident: Internal database ident of the heartbeat.
 * @message: Pointer to an idmef message object where the retrieved message will be stored.
 *
 * Returns: 0 on success or a negative value if an error occur.
 */
int requiemdb_get_heartbeat(requiemdb_t *db, uint64_t ident, idmef_message_t **message)
{
        return db->plugin->get_heartbeat(db->sql, ident, message);
}


/**
 * requiemdb_delete_alert:
 * @db: Pointer to a db object.
 * @ident: Internal database ident of the alert.
 *
 * Delete an alert.
 *
 * Returns: 0 on success, or a negative value if an error occur.
 */
int requiemdb_delete_alert(requiemdb_t *db, uint64_t ident)
{
        return db->plugin->delete_alert(db->sql, ident);
}


/**
 * requiemdb_delete_alert_from_list:
 * @db: Pointer to a db object.
 * @idents: Pointer to an array of idents.
 * @size: Size of @idents.
 *
 * Delete all alerts from ident stored within @idents.
 *
 * Returns: the number of alert deleted on success, or a negative value if an error occur.
 */
ssize_t requiemdb_delete_alert_from_list(requiemdb_t *db, uint64_t *ident, size_t size)
{
        if ( size == 0 )
                return 0;

        return _requiemdb_plugin_format_delete_alert_from_list(db->plugin, db->sql, ident, size);
}


/**
 * requiemdb_delete_alert_from_result_idents:
 * @db: Pointer to a db object.
 * @result: Pointer to an idents result object.
 *
 * Delete all alert from ident stored within @result.
 *
 * Returns: the number of alert deleted on success, or a negative value if an error occur.
 */
ssize_t requiemdb_delete_alert_from_result_idents(requiemdb_t *db, requiemdb_result_idents_t *result)
{
        return _requiemdb_plugin_format_delete_alert_from_result_idents(db->plugin, db->sql, result);
}



/**
 * requiemdb_delete_heartbeat:
 * @db: Pointer to a db object.
 * @ident: Internal database ident of the heartbeat.
 *
 * Delete an heartbeat.
 *
 * Returns: 0 on success, or a negative value if an error occur.
 */
int requiemdb_delete_heartbeat(requiemdb_t *db, uint64_t ident)
{
        return db->plugin->delete_heartbeat(db->sql, ident);
}


/**
 * requiemdb_delete_heartbeat_from_list:
 * @db: Pointer to a db object.
 * @idents: Pointer to an array of idents.
 * @size: Size of @idents.
 *
 * Delete all heartbeat from ident stored within @idents.
 *
 * Returns: the number of heartbeat deleted on success, or a negative value if an error occur.
 */
ssize_t requiemdb_delete_heartbeat_from_list(requiemdb_t *db, uint64_t *idents, size_t size)
{
        if ( size == 0 )
                return 0;

        return _requiemdb_plugin_format_delete_heartbeat_from_list(db->plugin, db->sql, idents, size);
}



/**
 * requiemdb_delete_heartbeat_from_result_idents:
 * @db: Pointer to a db object.
 * @result: Pointer to an idents result object.
 *
 * Delete all heartbeat from ident stored within @result.
 *
 * Returns: the number of heartbeat deleted on success, or a negative value if an error occur.
 */
ssize_t requiemdb_delete_heartbeat_from_result_idents(requiemdb_t *db, requiemdb_result_idents_t *result)
{
        return _requiemdb_plugin_format_delete_heartbeat_from_result_idents(db->plugin, db->sql, result);
}



/**
 * requiemdb_get_values:
 * @db: Pointer to a db object.
 * @path_selection: Pointer to a path selection.
 * @criteria: Pointer to a criteria object.
 * @distinct: Get distinct or not distinct result rows.
 * @limit: Limit of results or -1 if no limit.
 * @offset: Offset in results or -1 if no offset.
 * @result: Values result.
 *
 * Returns: the number of result or a negative value if an error occured.
 */
int requiemdb_get_values(requiemdb_t *db,
                         requiemdb_path_selection_t *path_selection,
                         idmef_criteria_t *criteria,
                         requiem_bool_t distinct,
                         int limit, int offset,
                         requiemdb_result_values_t **result)
{
        int ret;

        *result = calloc(1, sizeof (**result));
        if ( ! *result )
                return requiemdb_error_from_errno(errno);

        (*result)->db = db;
        (*result)->selection = path_selection;

        ret = db->plugin->get_values(db->sql, path_selection, criteria, distinct, limit, offset, &(*result)->res);
        if ( ret <= 0 )
                free(*result);

        return ret;
}



/**
 * requiemdb_transaction_start:
 * @db: Pointer to a #requiemdb_t object.
 *
 * Begin a transaction using @db object. Internal transaction
 * handling will be disabled until requiemdb_transaction_end()
 * or requiemdb_transaction_abort() is called.
 *
 * Returns: 0 on success or a negative value if an error occur.
 */
int requiemdb_transaction_start(requiemdb_t *db)
{
        int ret;

        ret = _requiemdb_sql_transaction_start(db->sql);
        if ( ret < 0 )
                return ret;

        _requiemdb_sql_disable_internal_transaction(db->sql);

        return ret;
}



/**
 * requiemdb_transaction_end:
 * @db: Pointer to a #requiemdb_t object.
 *
 * Terminate a sql transaction (SQL COMMIT command) initiated
 * with requiemdb_transaction_start(). Internal transaction
 * handling will be enabled again once this function return.
 *
 * Returns: 0 on success or a negative value if an error occur.
 */
int requiemdb_transaction_end(requiemdb_t *db)
{
        int ret;

        ret = _requiemdb_sql_transaction_end(db->sql);
        _requiemdb_sql_enable_internal_transaction(db->sql);

        if ( ret < 0 )
                return ret;

        return ret;
}



/**
 * requiemdb_transaction_abort:
 * @db: Pointer to a #requiemdb_t object.
 *
 * Abort a sql transaction (SQL ROLLBACK command) initiated
 * with requiemdb_transaction_start(). Internal transaction
 * handling will be enabled again once this function return.
 *
 * Returns: 0 on success or a negative value if an error occur.
 */
int requiemdb_transaction_abort(requiemdb_t *db)
{
        int ret;

        ret = _requiemdb_sql_transaction_abort(db->sql);
        _requiemdb_sql_enable_internal_transaction(db->sql);

        if ( ret < 0 )
                return ret;

        return ret;
}
