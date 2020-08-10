/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  handle.c - libtar code for initializing a TAR handle
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <sptk5/Exception.h>
#include "libtar.h"

#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;
using namespace sptk;

static tartype_t default_type = { (openfunc_t)open, (closefunc_t)close, (readfunc_t)read, (writefunc_t)write };

static TAR* tar_init(const char *pathname, tartype_t *type,int oflags, int /*mode*/, int options)
{
    if ((oflags & (O_RDWR|O_RDONLY|O_WRONLY)) == O_RDWR)
    {
        throw Exception("Invalid flags");
    }

    auto* t = new TAR;
    memset(t, 0, sizeof(TAR));

    t->pathname = pathname;
    t->options = options;
    t->type = (type ? type : &default_type);
    t->oflags = oflags;

    return t;
}


/* open a new tarfile handle */
TAR* tar_open(const char *pathname, tartype_t *type, int oflags, int mode, int options)
{
    auto* t = tar_init(pathname, type, oflags, mode, options);

    if ((options & TAR_NOOVERWRITE) && (oflags & O_CREAT))
        oflags |= O_EXCL;

#ifdef O_BINARY
    oflags |= O_BINARY;
#endif

    t->fd = (*(t->type->openfunc))(pathname, oflags, mode);
    if (t->fd == -1)
    {
        delete t;
        throw Exception("Can't open tar");
    }

    return t;
}


/* close tarfile handle */
int tar_close(TAR *t)
{
    int i;

    i = (*(t->type->closefunc))((int)t->fd);

    delete t;

    return i;
}
