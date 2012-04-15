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
#include <stdlib.h>

#include "requiemdb-error.h"


const char *requiemdb_strerror(requiemdb_error_t error)
{
        static const char *error_strings[] = {
                "Successful",
                "Unknown generic error",
                "Invalid SQL settings string",
                "Connection error",
                "Query error",
                "Invalid column num",
                "Invalid column name",
                "Invalid value",
                "Invalid value type",
                "Unknown sql plugin",
                "Unknown format plugin",
                "Already in transaction",
                "Not in transaction",
                "Invalid message ident",
                "Invalid selected path string",
                "Invalid path selection",
                "Database schema version too old",
                "Database schema version too recent",
                "Database schema version invalid",
                "Cannot load sql plugin",
                "Cannot load format plugin"
        };

        if ( requiem_error_is_verbose(error) )
                return _requiem_thread_get_error();

        if ( requiem_error_get_source(error) == REQUIEM_ERROR_SOURCE_REQUIEMDB ) {
                requiemdb_error_code_t code;

                code = requiem_error_get_code(error);
                if ( code >= sizeof(error_strings) / sizeof(*error_strings) )
                        return NULL;

                return error_strings[code];
        }

        return requiem_strerror(error);
}
