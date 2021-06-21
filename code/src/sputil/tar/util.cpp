/*
 **  Copyright 1998-2003 University of Illinois Board of Trustees
 **  Copyright 1998-2003 Mark D. Roth
 **  All rights reserved.
 **
 **  util.c - miscellaneous utility code for libtar
 **
 **  Mark D. Roth <roth@uiuc.edu>
 **  Campus Information Technologies and Educational Services
 **  University of Illinois at Urbana-Champaign
 */

#include "libtar.h"
#include <cstring>

#if defined(__GNUC__) || defined(__SUNPRO_C)

#include <sys/param.h>

#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 255
#endif

#define basename2(s) (strrchr(s,'/')==NULL?(char*)(s):(strrchr(s,'/')+1))
#define basename(s) (strrchr(s,'\\')==NULL?(basename2(s)):(strrchr(s,'\\')+1))

/* hashing function for pathnames */
int path_hashfunc(const char* key, int numbuckets)
{
    char buf[MAXPATHLEN + 1];
    const char* p;

    snprintf(buf, sizeof(buf), "%s", key);
    buf[MAXPATHLEN] = 0;
    p = basename(buf);

    return (((int) p[0]) % numbuckets);
}

int th_crc_calc(TAR* t)
{
    int i;
    int sum = 0;

    for (i = 0; i < T_BLOCKSIZE; ++i)
    {
        sum += ((unsigned char*) (&(t->th_buf)))[i];
    }

    for (i = 0; i < 8; ++i)
    {
        sum += (' ' - (unsigned char) t->th_buf.chksum[i]);
    }

    return sum;
}

/* string-octal to integer conversion */
int oct_to_int(const char* oct)
{
    unsigned i;

    if (sscanf(oct, "%o", &i) != 1)
    {
        i = 0;
    }

    return int(i);
}
