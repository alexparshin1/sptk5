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

#include "libtar.h"

#if defined(__GNUC__) || defined(__SUNPRO_C)
    #include <sys/param.h>
#endif
#include <cstring>

#ifndef MAXPATHLEN
#define MAXPATHLEN 255
#endif

/* determine full path name */
void th_get_pathname(TAR *t, char* path, size_t sz)
{
	if (t->th_buf.gnu_longname != nullptr) {
		strncpy(path, t->th_buf.gnu_longname, sz - 1);
		return;
	}

    if (t->th_buf.prefix[0] != '\0')
    {
#ifdef _MSC_VER
		int len = _snprintf(path, sz - 1, "%.155s/%.100s",t->th_buf.prefix, t->th_buf.name);
#else
		int len = snprintf(path, sz - 1, "%.155s/%.100s",t->th_buf.prefix, t->th_buf.name);
#endif
		path[len] = 0;
		return;
    }

#ifdef _MSC_VER
    int len = _snprintf(path, sz - 1, "%.100s", t->th_buf.name);
#else
    int len = snprintf(path, sz - 1, "%.100s", t->th_buf.name);
#endif
    path[len] = 0;
}

