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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <librequiem/requiem-log.h>
#include <librequiem/idmef.h>
#include <librequiem/idmef-tree-wrap.h>

#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb.h"

#include "classic-delete.h"


static int delete_message(requiemdb_sql_t *sql, unsigned int count, const char **queries, const char *idents)
{
        unsigned int i;
        int ret, tmp;

        ret = requiemdb_sql_transaction_start(sql);
        if ( ret < 0 )
                return ret;

        for ( i = 0; i < count; i++ ) {

                ret = requiemdb_sql_query_sprintf(sql, NULL, queries[i], idents);
                if ( ret < 0 )
                        goto error;
        }

        return requiemdb_sql_transaction_end(sql);

 error:
        tmp = requiemdb_sql_transaction_abort(sql);

        return (tmp < 0) ? tmp : ret;

}


static int do_delete_alert(requiemdb_sql_t *sql, const char *idents)
{
        static const char *queries[] = {
                "DELETE FROM Requiem_Action WHERE _message_ident %s",
                "DELETE FROM Requiem_AdditionalData WHERE _message_ident %s AND _parent_type = 'A'",
                "DELETE FROM Requiem_Address WHERE _message_ident %s AND _parent_type != 'H'",
                "DELETE FROM Requiem_Alert WHERE _ident %s",
                "DELETE FROM Requiem_Alertident WHERE _message_ident %s",
                "DELETE FROM Requiem_Analyzer WHERE _message_ident %s AND _parent_type = 'A'",
                "DELETE FROM Requiem_AnalyzerTime WHERE _message_ident %s AND _parent_type = 'A'",
                "DELETE FROM Requiem_Assessment WHERE _message_ident %s",
                "DELETE FROM Requiem_Classification WHERE _message_ident %s",
                "DELETE FROM Requiem_Reference WHERE _message_ident %s",
                "DELETE FROM Requiem_Confidence WHERE _message_ident %s",
                "DELETE FROM Requiem_CorrelationAlert WHERE _message_ident %s",
                "DELETE FROM Requiem_CreateTime WHERE _message_ident %s AND _parent_type = 'A'",
                "DELETE FROM Requiem_DetectTime WHERE _message_ident %s",
                "DELETE FROM Requiem_File WHERE _message_ident %s",
                "DELETE FROM Requiem_FileAccess WHERE _message_ident %s",
                "DELETE FROM Requiem_FileAccess_Permission WHERE _message_ident %s",
                "DELETE FROM Requiem_Impact WHERE _message_ident %s",
                "DELETE FROM Requiem_Inode WHERE _message_ident %s",
                "DELETE FROM Requiem_Checksum WHERE _message_ident %s",
                "DELETE FROM Requiem_Linkage WHERE _message_ident %s",
                "DELETE FROM Requiem_Node WHERE _message_ident %s AND _parent_type != 'H'",
                "DELETE FROM Requiem_OverflowAlert WHERE _message_ident %s",
                "DELETE FROM Requiem_Process WHERE _message_ident %s AND _parent_type != 'H'",
                "DELETE FROM Requiem_ProcessArg WHERE _message_ident %s AND _parent_type != 'H'",
                "DELETE FROM Requiem_ProcessEnv WHERE _message_ident %s AND _parent_type != 'H'",
                "DELETE FROM Requiem_SnmpService WHERE _message_ident %s",
                "DELETE FROM Requiem_Service WHERE _message_ident %s",
                "DELETE FROM Requiem_Source WHERE _message_ident %s",
                "DELETE FROM Requiem_Target WHERE _message_ident %s",
                "DELETE FROM Requiem_ToolAlert WHERE _message_ident %s",
                "DELETE FROM Requiem_User WHERE _message_ident %s",
                "DELETE FROM Requiem_UserId WHERE _message_ident %s",
                "DELETE FROM Requiem_WebService WHERE _message_ident %s",
                "DELETE FROM Requiem_WebServiceArg WHERE _message_ident %s"
        };

        return delete_message(sql, sizeof(queries) / sizeof(*queries), queries, idents);
}



static int do_delete_heartbeat(requiemdb_sql_t *sql, const char *idents)
{
        static const char *queries[] = {
                "DELETE FROM Requiem_AdditionalData WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_Address WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_Analyzer WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_AnalyzerTime WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_CreateTime WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_Node WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_Process WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_ProcessArg WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_ProcessEnv WHERE _parent_type = 'H' AND _message_ident %s",
                "DELETE FROM Requiem_Heartbeat WHERE _ident %s",
        };

        return delete_message(sql, sizeof(queries) / sizeof(*queries), queries, idents);
}


static ssize_t get_string_from_ident_list(requiem_string_t **out, uint64_t *ident, size_t size)
{
        int ret;
        size_t i;
        requiem_bool_t need_sep = FALSE;

        ret = requiem_string_new(out);
        if ( ret < 0 )
                return ret;

        ret = requiem_string_cat(*out, "IN (");
        if ( ret < 0 )
                goto err;

        for ( i = 0; i < size; i++ ) {

                ret = requiem_string_sprintf(*out, "%s%" REQUIEM_PRIu64, (need_sep) ? ", " : "", ident[i]);
                if ( ret < 0 )
                        goto err;

                need_sep = TRUE;
        }

        ret = requiem_string_cat(*out, ")");
        if ( ret < 0 )
                goto err;

        return i;

 err:
        requiem_string_destroy(*out);
        return ret;
}



static ssize_t get_string_from_result_ident(requiem_string_t **out, requiemdb_result_idents_t *res)
{
        int ret;
        size_t i = 0;
        uint64_t ident;
        requiem_bool_t need_sep = FALSE;

        ret = requiem_string_new(out);
        if ( ret < 0 )
                return ret;

        ret = requiem_string_cat(*out, "IN (");
        if ( ret < 0 )
                goto err;

        while ( requiemdb_result_idents_get_next(res, &ident) ) {

                ret = requiem_string_sprintf(*out, "%s%" REQUIEM_PRIu64, (need_sep) ? ", " : "", ident);
                if ( ret < 0 )
                        goto err;

                i++;
                need_sep = TRUE;
        }

        if ( i == 0 ) {
                ret = 0;
                goto err;
        }

        ret = requiem_string_cat(*out, ")");
        if ( ret < 0 )
                goto err;

        return i;

 err:
        requiem_string_destroy(*out);
        return ret;
}



int classic_delete_alert(requiemdb_sql_t *sql, uint64_t ident)
{
        char buf[32];

        snprintf(buf, sizeof(buf), "= %" REQUIEM_PRIu64, ident);

        return do_delete_alert(sql, buf);
}



int classic_delete_heartbeat(requiemdb_sql_t *sql, uint64_t ident)
{
        char buf[32];

        snprintf(buf, sizeof(buf), "= %" REQUIEM_PRIu64, ident);

        return do_delete_heartbeat(sql, buf);
}



ssize_t classic_delete_alert_from_result_idents(requiemdb_sql_t *sql, requiemdb_result_idents_t *results)
{
        int ret;
        ssize_t count;
        requiem_string_t *buf;

        count = get_string_from_result_ident(&buf, results);
        if ( count <= 0 )
                return count;

        ret = do_delete_alert(sql, requiem_string_get_string(buf));
        requiem_string_destroy(buf);

        return (ret < 0) ? ret : count;
}


ssize_t classic_delete_alert_from_list(requiemdb_sql_t *sql, uint64_t *ident, size_t size)
{
        int ret;
        ssize_t count;
        requiem_string_t *buf;

        count = get_string_from_ident_list(&buf, ident, size);
        if ( count < 0 )
                return count;

        ret = do_delete_alert(sql, requiem_string_get_string(buf));
        requiem_string_destroy(buf);

        return (ret < 0) ? ret : count;
}



ssize_t classic_delete_heartbeat_from_result_idents(requiemdb_sql_t *sql, requiemdb_result_idents_t *results)
{
        int ret;
        ssize_t count;
        requiem_string_t *buf;

        count = get_string_from_result_ident(&buf, results);
        if ( count <= 0 )
                return count;

        ret = do_delete_heartbeat(sql, requiem_string_get_string(buf));
        requiem_string_destroy(buf);

        return (ret < 0) ? ret : count;
}



ssize_t classic_delete_heartbeat_from_list(requiemdb_sql_t *sql, uint64_t *ident, size_t size)
{
        int ret;
        ssize_t count;
        requiem_string_t *buf;

        count = get_string_from_ident_list(&buf, ident, size);
        if ( count < 0 )
                return count;

        ret = do_delete_heartbeat(sql, requiem_string_get_string(buf));
        requiem_string_destroy(buf);

        return (ret < 0) ? ret : count;
}
