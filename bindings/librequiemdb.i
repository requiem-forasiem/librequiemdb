/*****
*
* Copyright (C) 2003-2005 PreludeIDS Technologies. All Rights Reserved.
* Author: Nicolas Delon 
*
* This file is part of the Requiem program.
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

#define inline 

%{
#include <librequiem/requiem.h>
#include <librequiem/idmef.h>

#include "requiemdb-sql-settings.h"
#include "requiemdb-sql.h"
#include "requiemdb-path-selection.h"
#include "requiemdb.h"
#include "requiemdb-version.h"
#include <librequiem/requiem-inttypes.h>
%}

typedef struct idmef_value idmef_value_t;
typedef struct idmef_criteria idmef_criteria_t;
typedef struct idmef_message idmef_message_t;
typedef struct idmef_time idmef_time_t;
typedef struct idmef_path idmef_path_t;
typedef struct idmef_criterion_value idmef_criterion_value_t;
typedef struct requiem_string requiem_string_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;

%ignore requiemdb_error_t;
typedef signed int requiemdb_error_t;

typedef enum { 
	REQUIEM_BOOL_TRUE = TRUE, 
	REQUIEM_BOOL_FALSE = FALSE 
} requiem_bool_t;


#ifdef SWIGPYTHON
%include librequiemdb_python.i
#endif /* ! SWIGPYTHON */

#ifdef SWIGPERL
%include librequiemdb_perl.i
#endif /* ! SWIGPERL */

%apply SWIGTYPE **OUTPARAM {
        idmef_message_t **,
        requiemdb_t **,
        requiemdb_path_selection_t **,
        requiemdb_result_idents_t **,
        requiemdb_result_values_t **,
        requiemdb_selected_path_t **,
        requiemdb_sql_t **,
        requiemdb_sql_settings_t **
};


%apply SWIGTYPE *INPARAM {
        idmef_message_t *,
        requiemdb_t *,
        requiemdb_path_selection_t *,
        requiemdb_result_idents_t *,
        requiemdb_result_values_t *,
        requiemdb_selected_path_t *,
        requiemdb_sql_t *,
        requiemdb_sql_field_t *,
        requiemdb_sql_row_t *,
        requiemdb_sql_settings_t *,
        requiemdb_sql_table_t *
};

%apply SWIGTYPE **OUTRESULT {
        requiemdb_result_idents_t **,
        requiemdb_result_values_t **,
        requiemdb_sql_field_t **,
        requiemdb_sql_row_t **,
        requiemdb_sql_table_t **
};


%include "requiemdb-sql-settings.h"
%include "requiemdb-sql.h"
%include "requiemdb.h"
%include "requiemdb-error.h"
%include "requiemdb-version.h"
%include "requiemdb-path-selection.h"
