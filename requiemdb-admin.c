/*****
*
* Copyright (C) 2005 PreludeIDS Technologies. All Rights Reserved.
* Author: Yoann Vandoorselaere <yoannv@gmail.com>
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
#include <signal.h>

#include <librequiem/idmef.h>
#include <librequiem/requiem.h>
#include <librequiem/idmef-message-print.h>

#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb-error.h"
#include "requiemdb-path-selection.h"
#include "requiemdb.h"


#define MAX_EVENT_PER_TRANSACTION 1000


/*
 * FIXME: cleanup the statistics handling mess.
 */

static sig_atomic_t stop_processing = 0;
static const char *query_logging = NULL;
static requiem_bool_t have_query_logging = FALSE;

static uint64_t cur_count = 0;
static int64_t limit = -1, offset = 0, offset_copy, limit_copy = -1;

static idmef_criteria_t *criteria = NULL;
static unsigned int events_per_transaction = MAX_EVENT_PER_TRANSACTION;


typedef struct {
        requiem_list_t list;
        double elapsed;
        size_t processed;
        const char *opname;
} stat_item_t;



/*
 * Runtime statistics.
 */
static REQUIEM_LIST(stat_list);
static struct timeval start, end;
static sig_atomic_t dump_stat = FALSE;



/*
 *
 */
ssize_t (*delete_message_from_result_idents)(requiemdb_t *db, requiemdb_result_idents_t *result) = NULL;
ssize_t (*delete_message_from_list)(requiemdb_t *db, uint64_t *idents, size_t size) = NULL;
int (*get_message)(requiemdb_t *db, uint64_t ident, idmef_message_t **message) = NULL;
int (*get_message_idents)(requiemdb_t *db, idmef_criteria_t *criteria,
                          int limit, int offset,
                          requiemdb_result_idents_order_t order,
                          requiemdb_result_idents_t **result) = NULL;


#define stat_compute(stat, op, count) stat_start(stat); op; stat_end(stat, count)


#ifndef MIN
# define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif


static stat_item_t *stat_item_new(const char *opname)
{
        stat_item_t *st;

        st = calloc(1, sizeof(*st));
        if ( ! st )
                return NULL;

        st->opname = opname;
        requiem_list_add_tail(&stat_list, &st->list);

        return st;
}



static void stat_dump_all(void)
{
        stat_item_t *stat;
        requiem_list_t *tmp;
        double total_elapsed = 0;
        size_t total_processed = 0;

        requiem_list_for_each(&stat_list, tmp) {
                stat = requiem_list_entry(tmp, stat_item_t, list);

                total_elapsed += stat->elapsed;
                total_processed = stat->processed;

                if ( limit != -1 )
                        fprintf(stderr, "%" REQUIEM_PRIu64 "/%" REQUIEM_PRIu64, (uint64_t) stat->processed, (uint64_t) limit);
                else
                        fprintf(stderr, "%" REQUIEM_PRIu64, (uint64_t) stat->processed);

                fprintf(stderr, " '%s' events processed in %f seconds (%f seconds/events - %f %s/sec average).\n",
                        stat->opname, stat->elapsed, stat->elapsed / stat->processed,
                        stat->processed / stat->elapsed, stat->opname);
        }

        if ( limit != -1 )
                fprintf(stderr, "%" REQUIEM_PRIu64 "/%" REQUIEM_PRIu64, (uint64_t) total_processed, (uint64_t) limit);
        else
                fprintf(stderr, "%" REQUIEM_PRIu64, (uint64_t) total_processed);

        fprintf(stderr, " events processed in %f seconds (%f seconds/events - %f events/sec average).\n",
                total_elapsed, total_elapsed / total_processed, total_processed / total_elapsed);
}


static void compute_elapsed(stat_item_t *stat, struct timeval *end, struct timeval *start)
{
        int nsec;

        /* Perform the carry for the later subtraction by updating 'start'. */
        if ( end->tv_usec < start->tv_usec ) {
                nsec = (start->tv_usec - end->tv_usec) / 1000000 + 1;
                start->tv_usec -= 1000000 * nsec;
                start->tv_sec += nsec;
        }

        if ( end->tv_usec - start->tv_usec > 1000000 ) {
                nsec = (end->tv_usec - start->tv_usec) / 1000000;
                start->tv_usec += 1000000 * nsec;
                start->tv_sec -= nsec;
        }

        /* Compute the time remaining to wait. tv_usec is certainly positive. */
        stat->elapsed += (end->tv_sec - start->tv_sec);
        stat->elapsed += ((float) (end->tv_usec - start->tv_usec) / 1000000);
}


static void stat_end(stat_item_t *stat, size_t count)
{
        gettimeofday(&end, NULL);

        stat->processed += count;
        compute_elapsed(stat, &end, &start);

        if ( ! dump_stat )
                return;

        stat_dump_all();
        dump_stat = FALSE;
}


static void stat_start(stat_item_t *stat)
{
        gettimeofday(&start, NULL);
}



static void free_global(void)
{
        if ( criteria )
                idmef_criteria_destroy(criteria);
}



static void flush_transaction_if_needed(requiemdb_t *db, unsigned int *event_no, unsigned int increment)
{
        if ( ! event_no )
                cur_count += increment;

        else if ( events_per_transaction ) {
                (*event_no) += increment;

                if ( *event_no >= events_per_transaction || (increment == 0 && *event_no) ) {
                        requiemdb_transaction_end(db);
                        requiemdb_transaction_start(db);

                        cur_count += *event_no;
                        *event_no = 0;
                }
        }
}


static void transaction_start(requiemdb_t *db)
{
        if ( events_per_transaction > 1 )
                requiemdb_transaction_start(db);
}


static void transaction_end(requiemdb_t *db, int ret, unsigned int event_no)
{
        if ( events_per_transaction > 1 ) {
                if ( ret < 0 )
                        requiemdb_transaction_abort(db);
                else {
                        cur_count += event_no;
                        requiemdb_transaction_end(db);
                }
        }
}



static void handle_stats_signal(int signo)
{
        /*
         * re-establish signal handler
         */
        signal(signo, handle_stats_signal);
        dump_stat = TRUE;
}



static void handle_signal(int signo)
{
        char buf[] = "Interrupted by signal. Will exit after current transaction.\n";

        /*
         * re-establish signal handler
         */
        signal(signo, handle_signal);

        write(STDERR_FILENO, buf, sizeof(buf));
        stop_processing = TRUE;
}


static int db_error(requiemdb_t *db, ssize_t ret, const char *fmt, ...)
{
        va_list ap;

        if ( ret == 0 )
                return ret;

        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);

        fprintf(stderr, ": %s.\n", requiemdb_strerror(ret));

        return ret;
}



static int fetch_message_idents_limited(requiemdb_t *db, requiemdb_result_idents_t **result, requiem_bool_t no_increment)
{
        int count;
        int64_t local_limit;

        local_limit = (limit_copy < 0) ? events_per_transaction : MIN(events_per_transaction, limit_copy);

        count = get_message_idents(db, criteria, (int) local_limit, (int) offset + ((no_increment) ? 0 : cur_count), 0, result);
        if ( count < 0 )
                return db_error(db, count, "retrieving alert ident failed");

        if ( limit_copy > 0 )
                limit_copy -= count;

        return count;
}


static int set_criteria(requiem_option_t *opt, const char *optarg, requiem_string_t *err, void *context)
{
        int ret;

        ret = idmef_criteria_new_from_string(&criteria, optarg);
        if ( ret < 0 )
                requiem_perror(ret, "could not load criteria");

        return ret;
}



static int set_query_logging(requiem_option_t *opt, const char *optarg, requiem_string_t *err, void *context)
{
        have_query_logging = TRUE;

        if ( optarg && strcmp(optarg, "-") != 0 )
                query_logging = strdup(optarg);

        return 0;
}


static int set_count(requiem_option_t *opt, const char *optarg, requiem_string_t *err, void *context)
{
        limit = strtoll(optarg, NULL, 0);
        if ( limit == REQUIEM_INT64_MIN || limit == REQUIEM_INT64_MAX ) {
                fprintf(stderr, "Invalid count specified: '%s'.\n", optarg);
                return -1;
        }

        limit_copy = limit;

        return 0;
}


static int set_offset(requiem_option_t *opt, const char *optarg, requiem_string_t *err, void *context)
{
        offset = strtoll(optarg, NULL, 0);
        if ( offset == REQUIEM_INT64_MIN || offset == REQUIEM_INT64_MAX ) {
                fprintf(stderr, "Invalid offset specified: '%s'.\n", optarg);
                return -1;
        }

        if ( offset < 0 )
                offset = 0;

        return 0;
}


static int set_events_per_transaction(requiem_option_t *opt, const char *optarg, requiem_string_t *err, void *context)
{
        events_per_transaction = atoi(optarg);
        return 0;
}


static int set_help(requiem_option_t *opt, const char *optarg, requiem_string_t *err, void *context)
{
        return requiem_error(REQUIEM_ERROR_EOF);
}



static void cmd_generic_help(void)
{
        fprintf(stderr, "Database arguments:\n");
        fprintf(stderr, "  host\t: Host where the database engine listen (default:localhost).\n");
        fprintf(stderr, "  port\t: Port where the database engine listen.\n");
        fprintf(stderr, "  type\t: Type of database (mysql/pgsql/sqlite).\n");
        fprintf(stderr, "  name\t: Name of the database.\n");
        fprintf(stderr, "  file\t: file where the database is stored (sqlite).\n");
        fprintf(stderr, "  user\t: User to access the database.\n");
        fprintf(stderr, "  pass\t: Password to access the database.\n\n");

        fprintf(stderr, "Valid options:\n");
        fprintf(stderr, "  --offset <offset>               : Skip processing until 'offset' events.\n");
        fprintf(stderr, "  --count <count>                 : Process at most count events.\n");
        fprintf(stderr, "  --query-logging [filename]      : Log SQL query to the specified file.\n");
        fprintf(stderr, "  --criteria <criteria>           : Only process events matching criteria.\n");
        fprintf(stderr, "  --events-per-transaction        : Maximum number of event to process per transaction (default %d).\n",
                events_per_transaction);
}



static void cmd_delete_help(void)
{
        fprintf(stderr, "Usage  : delete <alert|heartbeat> <database> [options]\n");
        fprintf(stderr, "Example: requiemdb-admin delete alert \"type=mysql name=dbname user=requiem\"\n\n");

        fprintf(stderr, "Delete messages from <database>.\n\n");

        cmd_generic_help();
}



static void cmd_save_help(void)
{
        fprintf(stderr, "Usage  : save <alert|heartbeat> <database> [filename] [options]\n");
        fprintf(stderr, "Example: requiemdb-admin save alert \"type=mysql name=dbname user=requiem\" outputfile\n\n");

        fprintf(stderr, "Save messages from <database> into [filename].\n");
        fprintf(stderr, "If no filename argument is provided, data will be written to standard output.\n\n");

        cmd_generic_help();
}



static void cmd_load_help(void)
{
        fprintf(stderr, "Usage  : load <database> [filename] [options]\n");
        fprintf(stderr, "Example: requiemdb-admin load \"type=mysql name=dbname user=requiem\" inputfile\n\n");

        fprintf(stderr, "Load messages from [filename] into <database>.\n");
        fprintf(stderr, "If no filename argument is provided, data will be read from standard input.\n\n");

        cmd_generic_help();
}


static void cmd_copy_move_help(requiem_bool_t delete_copied)
{
        fprintf(stderr, "Usage  : %s <alert|heartbeat> <database1> <database2> [options]\n", delete_copied ? "move" : "copy");
        fprintf(stderr, "Example: requiemdb-admin %s alert \"type=mysql name=dbname user=requiem\" \"type=pgsql name=dbname user=requiem\"\n\n", delete_copied ? "move" : "copy");

        fprintf(stderr, "%s messages from <database1> to <database2>.\n\n", delete_copied ? "Move" : "Copy");

        cmd_generic_help();
}



static void cmd_print_help(void)
{
        fprintf(stderr, "Usage  : print <alert|heartbeat> <database> [filename] [options]\n");
        fprintf(stderr, "Example: requiemdb-admin print alert \"type=mysql name=dbname user=requiem\"\n\n");

        fprintf(stderr, "Retrieve and print message from <database>.\n");
        fprintf(stderr, "Data will be written to [filename] if provided.\n\n");

        cmd_generic_help();
}



static void cmd_count_help(void)
{
        fprintf(stderr, "Usage  : count <alert|heartbeat> <database> [options]\n");
        fprintf(stderr, "Example: requiemdb-admin count alert \"type=mysql name=dbname user=requiem\"\n\n");

        fprintf(stderr, "Retrieve event count from <database>.\n");

        cmd_generic_help();
}



static void print_help(char **argv)
{
        fprintf(stderr, "Usage: %s <count|copy|delete|load|move|print|save> <arguments>\n\n", argv[0]);

        fprintf(stderr, "\tcount  - Retrieve event count from the database.\n");
        fprintf(stderr, "\tcopy   - Make a copy of a Requiem database to another database.\n");
        fprintf(stderr, "\tdelete - Delete content of a Requiem database.\n");
        fprintf(stderr, "\tload   - Load a Requiem database from a file.\n");
        fprintf(stderr, "\tmove   - Move content of a Requiem database to another database.\n");
        fprintf(stderr, "\tprint  - Print message from a Requiem database.\n");
        fprintf(stderr, "\tsave   - Save a Requiem database to a file.\n\n");
}





static int setup_generic_options(int *argc, char **argv)
{
        int ret;
        requiem_string_t *err;

        requiem_option_add(NULL, NULL, REQUIEM_OPTION_TYPE_CLI, 0, "offset",
                           NULL, REQUIEM_OPTION_ARGUMENT_REQUIRED, set_offset, NULL);

        requiem_option_add(NULL, NULL, REQUIEM_OPTION_TYPE_CLI, 0, "count",
                           NULL, REQUIEM_OPTION_ARGUMENT_REQUIRED, set_count, NULL);

        requiem_option_add(NULL, NULL, REQUIEM_OPTION_TYPE_CLI, 0, "query-logging",
                           NULL, REQUIEM_OPTION_ARGUMENT_OPTIONAL, set_query_logging, NULL);

        requiem_option_add(NULL, NULL, REQUIEM_OPTION_TYPE_CLI, 0, "criteria",
                           NULL, REQUIEM_OPTION_ARGUMENT_REQUIRED, set_criteria, NULL);

        requiem_option_add(NULL, NULL, REQUIEM_OPTION_TYPE_CLI, 0, "events-per-transaction",
                           NULL, REQUIEM_OPTION_ARGUMENT_REQUIRED, set_events_per_transaction, NULL);

        requiem_option_add(NULL, NULL, REQUIEM_OPTION_TYPE_CLI, 'h', "help",
                           NULL, REQUIEM_OPTION_ARGUMENT_NONE, set_help, NULL);

        ret = requiem_option_read(NULL, NULL, argc, argv, &err, NULL);
        if ( ret < 0 && requiem_error_get_code(ret) != REQUIEM_ERROR_EOF )
                fprintf(stderr, "Option error: %s.\n", requiem_strerror(ret));
        else
                *argc -= (ret - 1);

        return ret;
}


static int setup_message_type(const char *type)
{
        if ( strcasecmp(type, "alert") == 0 ) {
                get_message = requiemdb_get_alert;
                get_message_idents = requiemdb_get_alert_idents;
                delete_message_from_list = requiemdb_delete_alert_from_list;
                delete_message_from_result_idents = requiemdb_delete_alert_from_result_idents;
        }

        else if ( strcasecmp(type, "heartbeat") == 0 ) {
                get_message = requiemdb_get_heartbeat;
                get_message_idents = requiemdb_get_heartbeat_idents;
                delete_message_from_list = requiemdb_delete_heartbeat_from_list;
                delete_message_from_result_idents = requiemdb_delete_heartbeat_from_result_idents;
        }

        else {
                fprintf(stderr, "Invalid IDMEF message type: '%s', should be \"alert\" or \"heartbeat\".\n", type);
                return -1;
        }

        return 0;
}


static int db_new_from_string(requiemdb_t **db, const char *str)
{
        int ret;
        requiemdb_sql_t *sql;
        requiemdb_sql_settings_t *sql_settings;

        ret = requiemdb_sql_settings_new_from_string(&sql_settings, str);
        if ( ret < 0 ) {
                fprintf(stderr, "Error loading database settings: %s.\n", requiemdb_strerror(ret));
                return ret;
        }

        ret = requiemdb_sql_new(&sql, NULL, sql_settings);
        if ( ret < 0 ) {
                fprintf(stderr, "Error creating database interface: %s.\n", requiemdb_strerror(ret));
                requiemdb_sql_settings_destroy(sql_settings);
                return ret;
        }

        ret = requiemdb_new(db, sql, NULL, NULL, 0);
        if ( ret < 0 ) {
                fprintf(stderr, "could not initialize database '%s': %s\n", str, requiemdb_strerror(ret));
                requiemdb_sql_destroy(sql);
                return ret;
        }

        if ( have_query_logging )
                requiemdb_sql_enable_query_logging(sql, query_logging);

        return 0;
}



static int do_delete(requiemdb_t *db, requiemdb_result_idents_t *idents,
                     ssize_t (*dfunc)(requiemdb_t *db, requiemdb_result_idents_t *idents),
                     stat_item_t *stat_delete)
{
        ssize_t ret;

        stat_compute(stat_delete, ret = dfunc(db, idents), ret);
        if ( ret < 0 )
                ret = db_error(db, ret, "delete event failed");

        return ret;
}



static int copy_iterate(requiemdb_t *src, requiemdb_t *dst,
                        requiemdb_result_idents_t *idents,
                        unsigned int *dst_event_no,
                        int (*get_message)(requiemdb_t *db, uint64_t ident, idmef_message_t **msg),
                        ssize_t (*delete)(requiemdb_t *db, uint64_t *idents, size_t size),
                        stat_item_t *stat_fetch, stat_item_t *stat_insert, stat_item_t *stat_delete)
{
        int ret = 0;
        ssize_t count;
        uint64_t ident;
        idmef_message_t *msg;
        size_t delete_index = 0;
        uint64_t delete_tbl[1024];

        while ( (ret = requiemdb_result_idents_get_next(idents, &ident)) > 0 ) {

                stat_compute(stat_fetch, ret = get_message(src, ident, &msg), 1);
                if ( ret < 0 ) {
                        db_error(src, ret, "Error retrieving message ident %" REQUIEM_PRIu64 "", ident);
                        return ret;
                }

                stat_compute(stat_insert, ret = requiemdb_insert_message(dst, msg), 1);
                idmef_message_destroy(msg);

                if ( ret < 0 ) {
                        db_error(dst, ret, "Error inserting message %" REQUIEM_PRIu64 "", ident);
                        return ret;
                }

                if ( delete ) {
                        delete_tbl[delete_index++] = ident;

                        if ( (events_per_transaction && *dst_event_no >= events_per_transaction) ||
                              delete_index >= (sizeof(delete_tbl) / sizeof(*delete_tbl)) ) {

                                stat_compute(stat_delete, count = delete(src, delete_tbl, delete_index), count);
                                if ( count < 0 ) {
                                        db_error(dst, count, "Error deleting %d messages", delete_index);
                                        continue;
                                }

                                delete_index = 0;
                        }
                }

                flush_transaction_if_needed(dst, dst_event_no, 1);
        }

        flush_transaction_if_needed(dst, dst_event_no, 0);

        if ( delete_index ) {
                stat_compute(stat_delete, count = delete(src, delete_tbl, delete_index), count);
                if ( count < 0 ) {
                        db_error(dst, count, "Error deleting message");
                        return -1;
                }
        }

        return ret;
}



static int do_cmd_copy_move(int argc, char **argv, requiem_bool_t delete_copied)
{
        int ret, idx, count;
        requiemdb_t *src, *dst;
        requiemdb_result_idents_t *idents;
        unsigned int dst_event_no = 0;
        stat_item_t *stat_delete = NULL;
        stat_item_t *stat_fetch = stat_item_new("fetch");
        stat_item_t *stat_insert = stat_item_new("insert");

        idx = setup_generic_options(&argc, argv);
        if ( idx < 0 || argc < 3 ) {
                cmd_copy_move_help(delete_copied);
                exit(1);
        }

        ret = setup_message_type(argv[idx++]);
        if ( ret < 0 ) {
                cmd_copy_move_help(delete_copied);
                exit(1);
        }

        ret = db_new_from_string(&src, argv[idx]);
        if ( ret < 0 )
                return ret;

        ret = db_new_from_string(&dst, argv[idx + 1]);
        if ( ret < 0 )
                return ret;

        if ( delete_copied )
                stat_delete = stat_item_new("delete");

        transaction_start(dst);

        do {
                count = ret = fetch_message_idents_limited(src, &idents, (delete_copied) ? TRUE : FALSE);
                if ( count > 0 ) {
                        ret = copy_iterate(src, dst, idents, &dst_event_no, get_message,
                                           (delete_copied) ? delete_message_from_list : NULL,
                                           stat_fetch, stat_insert, stat_delete);
                        requiemdb_result_idents_destroy(idents);
                }
        } while ( count > 0 && ret >= 0 && ! stop_processing );

        transaction_end(dst, ret, dst_event_no);

        requiemdb_destroy(src);
        requiemdb_destroy(dst);

        return ret;
}


static int cmd_copy(int argc, char **argv)
{
        return do_cmd_copy_move(argc, argv, FALSE);
}



static int cmd_move(int argc, char **argv)
{
        return do_cmd_copy_move(argc, argv, TRUE);
}



static int cmd_delete(int argc, char **argv)
{
        requiemdb_t *db;
        int ret, idx, count;
        unsigned int event_no = 0;
        requiemdb_result_idents_t *idents;
        stat_item_t *stat_delete = stat_item_new("delete");

        idx = setup_generic_options(&argc, argv);
        if ( idx < 0 || argc != 3 ) {
                cmd_delete_help();
                exit(1);
        }

        ret = setup_message_type(argv[idx++]);
        if ( ret < 0 ) {
                cmd_delete_help();
                exit(1);
        }

        if ( ! criteria ) {
                fprintf(stderr, "No criteria provided for deletion.\n");
                return -1;
        }

        ret = db_new_from_string(&db, argv[idx]);
        if ( ret < 0 )
                return ret;

        transaction_start(db);

        do {
                count = ret = fetch_message_idents_limited(db, &idents, TRUE);
                if ( count > 0 ) {
                        ret = do_delete(db, idents, delete_message_from_result_idents, stat_delete);
                        requiemdb_result_idents_destroy(idents);

                        flush_transaction_if_needed(db, &event_no, count);
                }

        } while ( count > 0 && ret >= 0 && ! stop_processing );

        transaction_end(db, ret, event_no);

        requiemdb_destroy(db);
        return ret;
}



static int save_msg(requiem_msgbuf_t *msgbuf, requiem_msg_t *msg)
{
        int ret;
        requiem_io_t *fd = requiem_msgbuf_get_data(msgbuf);

        ret = requiem_msg_write(msg, fd);
        requiem_msg_recycle(msg);

        return ret;
}



static int save_iterate_message(requiemdb_t *db, requiemdb_result_idents_t *idents, requiem_msgbuf_t *msgbuf,
                                int (*get_message)(requiemdb_t *db, uint64_t ident, idmef_message_t **out),
                                stat_item_t *stat_fetch, stat_item_t *stat_save)
{
        int ret = 0;
        uint64_t ident;
        idmef_message_t *message;

        while ( ! stop_processing && (ret = requiemdb_result_idents_get_next(idents, &ident)) > 0 ) {

                stat_compute(stat_fetch, ret = get_message(db, ident, &message), 1);
                if ( ret < 0 ) {
                        db_error(db, ret, "Error retrieving message ident %" REQUIEM_PRIu64 "", ident);
                        continue;
                }

                stat_compute(stat_save, ret = idmef_message_write(message, msgbuf); requiem_msgbuf_mark_end(msgbuf), 1);
                idmef_message_destroy(message);

                if ( ret < 0 ) {
                        requiem_perror(ret, "Error writing message %" REQUIEM_PRIu64 " to disk", ident);
                        continue;
                }

                cur_count++;
        }

        return 0;
}



static int cmd_save(int argc, char **argv)
{
        int ret, idx, count;
        requiemdb_t *db;
        requiem_io_t *io;
        FILE *fd = stdout;
        requiem_msgbuf_t *msgbuf;
        requiemdb_result_idents_t *idents;
        stat_item_t *stat_fetch = stat_item_new("fetch");
        stat_item_t *stat_save = stat_item_new("save");

        idx = setup_generic_options(&argc, argv);
        if ( idx < 0 || argc < 3 ) {
                cmd_save_help();
                exit(1);
        }

        ret = setup_message_type(argv[idx++]);
        if ( ret < 0 ) {
                cmd_save_help();
                exit(1);
        }

        ret = db_new_from_string(&db, argv[idx]);
        if ( ret < 0 )
                return ret;

        if ( argc > 3 && *argv[idx + 1] != '-' ) {
                fd = fopen(argv[idx + 1], "w");
                if ( ! fd ) {
                        fprintf(stderr, "could not open '%s' for writing: %s.\n", argv[idx + 1], strerror(errno));
                        return -1;
                }
        }

        ret = requiem_io_new(&io);
        if ( ret < 0 )
                return ret;

        requiem_io_set_file_io(io, fd);

        ret = requiem_msgbuf_new(&msgbuf);
        if ( ret < 0 )
                return ret;

        requiem_msgbuf_set_data(msgbuf, io);
        requiem_msgbuf_set_callback(msgbuf, save_msg);

        do {
                count = ret = fetch_message_idents_limited(db, &idents, FALSE);
                if ( count > 0 ) {
                        ret = save_iterate_message(db, idents, msgbuf, get_message, stat_fetch, stat_save);
                        requiemdb_result_idents_destroy(idents);
                }

        } while ( count > 0 && ret >= 0 && ! stop_processing );

        requiem_msgbuf_destroy(msgbuf);

        if ( fd != stdout )
                requiem_io_close(io);

        requiemdb_destroy(db);

        return ret;
}



static int do_read_message(requiem_io_t *io, requiem_msg_t **msg, idmef_message_t **idmef)
{
        int ret;

        *msg = NULL;
        *idmef = NULL;

        ret = requiem_msg_read(msg, io);
        if ( ret < 0 ) {
                if ( requiem_error_get_code(ret) == REQUIEM_ERROR_EOF )
                        return 0;

                return ret;
        }

        if ( offset_copy > 0 ) {
                offset_copy--;
                requiem_msg_destroy(*msg);
                return 1;
        }

        if ( limit != -1 && cur_count >= (uint64_t) limit ) {
                requiem_msg_destroy(*msg);
                return 0;
        }

        ret = idmef_message_new(idmef);
        if ( ret < 0 ) {
                fprintf(stderr, "error creating new IDMEF message: %s.\n", requiem_strerror(ret));
                return ret;
        }

        ret = idmef_message_read(*idmef, *msg);
        if ( ret < 0 )
                return ret;

        return 1;
}


static int load_from_file(requiemdb_t *db, requiem_io_t *io, stat_item_t *stat_fetch,
                          stat_item_t *stat_insert, unsigned int *event_no)
{
        int ret = 0;
        requiem_msg_t *msg;
        idmef_message_t *idmef;

        while ( ! stop_processing ) {
                stat_start(stat_fetch);
                ret = do_read_message(io, &msg, &idmef);
                stat_end(stat_fetch, (ret >= 0) ? ret : 0);

                if ( ret < 0 ) {
                        fprintf(stderr, "error decoding IDMEF message: %s.\n", requiem_strerror(ret));
                        break;
                }

                if ( ret == 0 )
                        break;

                if ( ! idmef )
                        continue;

                if ( ! criteria || idmef_criteria_match(criteria, idmef) ) {
                        stat_compute(stat_insert, ret = requiemdb_insert_message(db, idmef), 1);
                        if ( ret < 0 ) {
                                db_error(db, ret, "error inserting IDMEF message");
                                break;
                        }
                }

                idmef_message_destroy(idmef);
                requiem_msg_destroy(msg);

                flush_transaction_if_needed(db, event_no, 1);
        }

        return ret;
}


static int cmd_load(int argc, char **argv)
{
        FILE *fd = stdin;
        requiemdb_t *db;
        requiem_io_t *io;
        int ret, idx, argc2;
        unsigned int event_no = 0;
        stat_item_t *stat_fetch = stat_item_new("fetch");
        stat_item_t *stat_insert = stat_item_new("insert");

        argc2 = argc;
        idx = setup_generic_options(&argc2, argv);
        if ( idx < 0 || argc < 2 ) {
                cmd_load_help();
                exit(1);
        }

        ret = db_new_from_string(&db, argv[idx++]);
        if ( ret < 0 )
                return ret;

        ret = requiem_io_new(&io);
        if ( ret < 0 )
                return ret;

        if ( idx == argc )
                argv[argc++] = "-";

        transaction_start(db);

        while ( idx < argc ) {

                fd = (strcmp(argv[idx], "-") == 0) ? stdin : fopen(argv[idx], "r");
                if ( ! fd ) {
                        fprintf(stderr, "could not open '%s' for reading: %s.\n", argv[idx], strerror(errno));
                        return -1;
                }

                requiem_io_set_file_io(io, fd);

                offset_copy = offset;

                ret = load_from_file(db, io, stat_fetch, stat_insert, &event_no);
                if ( ret < 0 ) {
                        fprintf(stderr, "error reading reading '%s': %s.\n", argv[idx], requiem_strerror(ret));
                        break;
                }

                if ( fd != stdin )
                        requiem_io_close(io);

                idx++;
        }

        transaction_end(db, ret, event_no);

        requiem_io_destroy(io);
        requiemdb_destroy(db);

        return ret;
}



static int print_iterate_message(requiemdb_t *db, requiemdb_result_idents_t *idents, requiem_io_t *io,
                                 int (*get_message)(requiemdb_t *db, uint64_t ident, idmef_message_t **idmef),
                                 stat_item_t *stat_fetch, stat_item_t *stat_print)
{
        int ret = 0;
        uint64_t ident;
        idmef_message_t *idmef;

        while ( ! stop_processing && (ret = requiemdb_result_idents_get_next(idents, &ident)) > 0 ) {

                stat_compute(stat_fetch, ret = get_message(db, ident, &idmef), 1);
                if ( ret < 0 ) {
                        db_error(db, ret, "Error retrieving message ident %" REQUIEM_PRIu64, ident);
                        return ret;
                }

                stat_compute(stat_print, idmef_message_print(idmef, io), 1);
                idmef_message_destroy(idmef);
                cur_count++;
        }

        return ret;
}



static int cmd_print(int argc, char **argv)
{
        int ret, idx, count;
        FILE *fd = stdout;
        requiemdb_t *db;
        requiem_io_t *io;
        requiemdb_result_idents_t *idents;
        stat_item_t *stat_fetch = stat_item_new("fetch");
        stat_item_t *stat_print = stat_item_new("print");

        idx = setup_generic_options(&argc, argv);
        if ( idx < 0 || argc < 3 ) {
                cmd_print_help();
                exit(1);
        }

        ret = setup_message_type(argv[idx++]);
        if ( ret < 0 ) {
                cmd_print_help();
                exit(1);
        }

        ret = db_new_from_string(&db, argv[idx]);
        if ( ret < 0 )
                return ret;

        if ( argc > 3 && *argv[idx + 1] != '-' ) {

                fd = fopen(argv[idx + 1], "w");
                if ( ! fd ) {
                        fprintf(stderr, "could not open '%s' for reading: %s.\n", argv[idx + 1], strerror(errno));
                        return -1;
                }
        }

        ret = requiem_io_new(&io);
        if ( ret < 0 )
                return ret;

        requiem_io_set_file_io(io, fd);

        do {
                count = ret = fetch_message_idents_limited(db, &idents, FALSE);
                if ( count > 0 ) {
                        ret = print_iterate_message(db, idents, io, get_message, stat_fetch, stat_print);
                        requiemdb_result_idents_destroy(idents);
                }
        } while ( count > 0 && ret >= 0 && ! stop_processing );

        if ( fd != stdout )
                requiem_io_close(io);

        requiem_io_destroy(io);
        requiemdb_destroy(db);

        return ret;
}



static int cmd_count(int argc, char **argv)
{
        char buf[128];
        int ret, idx;
        requiemdb_t *db;
        idmef_value_t **values;
        requiemdb_result_values_t *results;
        requiemdb_path_selection_t *ps;
        requiemdb_selected_path_t *sp;

        idx = setup_generic_options(&argc, argv);
        if ( idx < 0 || argc < 3 ) {
                cmd_count_help();
                exit(1);
        }

        snprintf(buf, sizeof(buf), "count(%s.create_time)", argv[idx]);

        ret = setup_message_type(argv[idx++]);
        if ( ret < 0 ) {
                cmd_count_help();
                exit(1);
        }

        ret = db_new_from_string(&db, argv[idx]);
        if ( ret < 0 )
                return ret;

        ret = requiemdb_path_selection_new(&ps);
        if ( ret < 0 ) {
                requiemdb_destroy(db);
                return ret;
        }

        ret = requiemdb_selected_path_new_string(&sp, buf);
        if ( ret < 0 ) {
                requiemdb_destroy(db);
                requiemdb_path_selection_destroy(ps);
                return ret;
        }

        requiemdb_path_selection_add(ps, sp);

        ret = requiemdb_get_values(db, ps, criteria, FALSE, (int) limit, (int) offset, &results);
        if ( ret < 0 ) {
                ret = db_error(db, ret, "error retrieving database count");
                goto err;
        }

        ret = requiemdb_result_values_get_next(results, &values);
        if ( ret < 0 )
                goto err;

        printf("COUNT: %u\n", idmef_value_get_uint32(values[0]));
        idmef_value_destroy(values[0]);

err:
        requiemdb_path_selection_destroy(ps);
        requiemdb_result_values_destroy(results);
        requiemdb_destroy(db);

        return ret;
}



int main(int argc, char **argv)
{
        int ret;
        size_t i;
        const struct {
                char *name;
                int (*run)(int argc, char **argv);
        } commands[] = {
                { "count", cmd_count   },
                { "copy", cmd_copy     },
                { "delete", cmd_delete },
                { "load", cmd_load     },
                { "move", cmd_move     },
                { "print", cmd_print   },
                { "save", cmd_save     },
        };

        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);
        signal(SIGUSR1, handle_stats_signal);
        signal(SIGQUIT, handle_stats_signal);

        ret = requiemdb_init();
        if ( ret < 0 ) {
                requiem_perror(ret, "error initializing librequiemdb");
                return ret;
        }

        if ( argc < 2 ) {
                print_help(argv);
                return -1;
        }

        for ( i = 0; i < sizeof(commands) / sizeof(*commands); i++ ) {
                if ( strcmp(argv[1], commands[i].name) != 0 )
                        continue;

                ret = commands[i].run(--argc, &argv[1]);

                if ( (ret < 0 || stop_processing) )
                        fprintf(stderr, "%s at transaction %" REQUIEM_PRIu64 ". Use --offset %" REQUIEM_PRIu64 " to resume operation.\n",
                                (ret < 0) ? "Error"  : "Interrupted by signal", cur_count, (offset == -1) ? cur_count : cur_count + offset);

                if ( cur_count > 0 || ret == 0 )
                        stat_dump_all();

                free_global();
                requiemdb_deinit();

                return ret;
        }

        fprintf(stderr, "Unknown command: %s.\n", argv[1]);
        print_help(argv);

        free_global();
        requiemdb_deinit();

        return -1;
}
