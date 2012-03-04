/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  decode.c - libtar code to decode tar header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <sptk5/sptk.h>
#include "libtar.h"

#include <stdio.h>
#if defined(__GNUC__) || defined(__SUNPRO_C)
#include <sys/param.h>
#endif
#include <string.h>


#ifndef MAXPATHLEN
#define MAXPATHLEN 255
#endif

/* determine full path name */
char *
th_get_pathname(TAR *t)
{
	char filename[MAXPATHLEN];

	if (t->th_buf.gnu_longname)
		return t->th_buf.gnu_longname;

	if (t->th_buf.prefix[0] != '\0')
	{
#ifdef _MSC_VER
		_snprintf(filename, sizeof(filename), "%.155s/%.100s",t->th_buf.prefix, t->th_buf.name);
#else
		snprintf(filename, sizeof(filename), "%.155s/%.100s",t->th_buf.prefix, t->th_buf.name);
#endif
		return strdup(filename);
	}

#ifdef _MSC_VER
	_snprintf(filename, sizeof(filename), "%.100s", t->th_buf.name);
#else
	snprintf(filename, sizeof(filename), "%.100s", t->th_buf.name);
#endif
	return strdup(filename);
}

