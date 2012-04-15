/*****
*
* Copyright (C) 2005 PreludeIDS Technologies. All Rights Reserved.
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

%module RequiemDB

%typemap(in) SWIGTYPE **OUTPARAM ($*1_type tmp) {
        $1 = ($1_ltype) &tmp;
};


%typemap(argout) SWIGTYPE **OUTPARAM {
        SV *sv;

        sv = SvRV($input);
        sv_setsv(sv, SWIG_NewPointerObj((void *) * $1, $*1_descriptor, 0));
};


%typemap(in) uint8_t {
	$1 = (uint8_t) SvIV($input);
};


%typemap(in) uint64_t {
	if ( SvIOK($input) ) {
		$1 = (uint64_t) SvIV($input);

	} else {
		if ( sscanf(SvPV_nolen($input), "%" REQUIEM_PRIu64, &($1)) < 1 ) {
			croak("Argument %s is not an unsigned 64 bits integer\n", SvPV_nolen($input));
		}
	}
};


/* %typemap(out) int { */
/*         $result = newSViv($1); */
/*         argvi++; */

/*         if ( $1 < 0 ) */
/*                 XSRETURN(argvi); */
/* }; */


%typemap(in) idmef_value_t ***values (idmef_value_t **tmp) {
	$1 = &tmp;
};


%typemap(argout) idmef_value_t ***values {
	I32 len;
	unsigned int cnt;
	AV *av;

	if ( SvTYPE(SvRV($input)) != SVt_PVAV )
		croak("want a reference to an array");

	av = (AV *) SvRV($input);

	len = av_len(av);

	for ( cnt = 0; cnt < len; cnt++ ) {
		av_push(av, SWIG_NewPointerObj((void *) (*$1)[cnt], $descriptor(idmef_value_t *), 0));
	}

	free($1);
};


%typemap(in) SWIGTYPE *ident (uint64_t tmp) {
	$1 = &tmp;
};


%typemap(argout) uint64_t *ident {
	if ( SvIV($result) == 0 )
		sv_setsv(SvRV($input), &PL_sv_undef);
	else {
		sv_setsv(SvRV($input), sv_2mortal(newSVpvf("%" REQUIEM_PRIu64, *$1)));
	}
};



%typemap(in) requiem_string_t *output {
	int ret;

	ret = requiem_string_new(&($1));
	if ( ret < 0 )
		croak("%s", requiem_strerror(ret));
};



%typemap(argout) requiem_string_t *output {
	sv_setsv(SvRV($input), newSVpv(requiem_string_get_string($1), 0));
	requiem_string_destroy($1);
};



%typemap(in) SWIGTYPE **OUTRESULT ($*1_type tmp) {
        $1 = ($1_ltype) &tmp;
};



%typemap(argout) SWIGTYPE **OUTRESULT {
	if ( SvIV(ST(argvi - 1)) > 0 ) {
		SV *sv;

		sv = SvRV($input);
		
		sv_setsv(sv, SWIG_NewPointerObj((void *) * $1, $*1_descriptor, 0));
	}
};


%typemap(in) SWIGTYPE *INPARAM {
        if ( ! SvROK($input) ) {
                croak("Argument $argnum is null.");
                return;
        }

        if ( SWIG_ConvertPtr($input, (void **)&arg$argnum, $1_descriptor, 0) )
                return;
}


%typemap(in) char **output (char *tmp) {
	$1 = &tmp;
};



%typemap(argout) char **output {
	if ( SvIV($result) < 0 )
		croak("%s", requiem_strerror(SvIV($result)));

	sv_setsv(SvRV($input), sv_2mortal(newSVpv(*$1, 0)));
	free(*$1);
};


